#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

const unsigned GNUPLOTPP_VERSION = 0x000800;
const unsigned GNUPLOTPP_MAJOR_VERSION = (GNUPLOTPP_VERSION & 0xFF0000) >> 16;
const unsigned GNUPLOTPP_MINOR_VERSION = (GNUPLOTPP_VERSION & 0x00FF00) >> 8;
const unsigned GNUPLOTPP_PATCH_VERSION = (GNUPLOTPP_VERSION & 0xFF);

class Gnuplot {
private:
	static FILE* safe_popen(const char* name, const char* mode) {
#ifdef _WIN32
		return _popen(name, mode);
#else
		return popen(name, mode);
#endif
	}

	static int safe_pclose(FILE* f) {
#ifdef _WIN32
		return _pclose(f);
#else
		return pclose(f);
#endif
	}

	static void safe_sleep(unsigned seconds) {
#ifdef _WIN32
		Sleep(seconds * 1000);
#else
		sleep(seconds);
#endif
	}

	std::string escape_quotes(const std::string& s) {
		std::string result{};

		for (char c : s) {
			if (c != '\'')
				result.push_back(c);
			else
				result += "''";
		}

		return result;
	}

	std::vector<double> list_of_x;
	std::vector<double> list_of_y;
	std::vector<double> list_of_xerr;
	std::vector<double> list_of_yerr;

	void check_consistency() const {
		assert(list_of_x.size() == list_of_y.size());

		if (list_of_xerr.size() > 0)
			assert(list_of_xerr.size() == list_of_x.size());
		if (list_of_yerr.size() > 0)
			assert(list_of_yerr.size() == list_of_y.size());
	}

	bool useAutoshow = true;

public:
	enum class LineStyle {
		DOTS,
		LINES,
		POINTS,
		LINESPOINTS,
		STEPS,
		BOXES,
		X_ERROR_BARS,
		Y_ERROR_BARS,
		XY_ERROR_BARS,
		VECTORS,
		PM3D,
	};

	enum class AxisScale {
		LINEAR,
		LOGX,
		LOGY,
		LOGXY,
	};

	enum class TerminalMode {
		MONO,
		ANSI,
		ANSI256,
		ANSIRGB,
	};

	Gnuplot(const char* executable_name = "gnuplot", bool persist = true)
		: connection{}, series{}, files_to_delete{}, is_3dplot{ false } {
		
		std::stringstream os;
		// The --persist flag lets Gnuplot keep running after the C++
		// program has completed its execution
		os << executable_name;
		if (persist)
			os << " --persist";
		connection = safe_popen(os.str().c_str(), "w");

		set_xrange();
		set_yrange();
		set_zrange();

#ifdef _WIN32
		sendcommand("\nset terminal windows font 'Trebuchet MS,12' linewidth 1.5\n");
		sendcommand("\nset encoding cp1251\n");
#else
		sendcommand("set terminal qt linewidth 1.5\n");
		sendcommand("set encoding koi8r\n");
#endif
		sendcommand("set minussign\n");
		sendcommand("set size ratio 0.8\n");
		sendcommand("set autoscale noextend\n");
		sendcommand("set decimalsign \",\"\n");
		sendcommand("set colorsequence classic");
		//sendcommand("set format \" % '.2f\"\n");
	}

	~Gnuplot() {
		if (useAutoshow) show();

		if (connection) {
			safe_pclose(connection);
			connection = nullptr;
		}

		// Let some time pass before removing the files, so that Gnuplot
		// can finish displaying the last plot.
		safe_sleep(1);

		// Now remove the data files
		for (const auto& fname : files_to_delete) {
			std::remove(fname.c_str());
		}
	}

	/* This is the most low-level method in the Gnuplot class! It
		   returns `true` if the send command was successful, `false`
		   otherwise. */
	bool sendcommand(const char* str) {
		if (connection == nullptr)
			return false;

		fputs(str, connection);
		fputc('\n', connection);
		fflush(connection);

		return true;
	}

	bool sendcommand(const std::string& str) { return sendcommand(str.c_str()); }
	bool sendcommand(const std::stringstream& stream) { return sendcommand(stream.str()); }

#pragma region SAVE AS METHODS

	bool save_as_png(const std::string& filename,
		const std::string& size = "800,600") {
		std::stringstream os;

		os << "set terminal pngcairo color enhanced size " << size << "\n"
			<< "set output '" << filename << "'\n";
		return sendcommand(os);
	}

	bool save_as_pdf(const std::string& filename,
		std::string size = "16cm,12cm") {
		std::stringstream os;

		os << "set terminal pdfcairo color enhanced size " << size << "\n"
			<< "set output '" << filename << "'\n";
		return sendcommand(os);
	}

	bool save_as_svg(const std::string& filename,
		const std::string& size = "800,600") {
		std::stringstream os;

		os << "set terminal svg enhanced mouse standalone size " << size << "\n"
			<< "set output '" << filename << "'\n";
		return sendcommand(os);
	}

	bool save_as_dumb(const std::string& filename = "",
		unsigned int width = 80,
		unsigned int height = 50,
		TerminalMode mode = TerminalMode::MONO) {
		std::stringstream os;

		os << "set terminal dumb size " << width << " " << height;

		switch (mode) {
		case TerminalMode::MONO: os << "mono"; break;
		case TerminalMode::ANSI: os << "ansi"; break;
		case TerminalMode::ANSI256: os << "ansi256"; break;
		case TerminalMode::ANSIRGB: os << "ansirgb"; break;
		default: os << "mono";
		}

		os << "\n";

		if (!filename.empty()) {
			os << "set output '" << filename << "'\n";
		}

		return sendcommand(os);
	}

#pragma endregion SAVE AS METHODS

#pragma region PLOT PROPERTIES SETTERS

	bool set_title(const std::string& title) {
		std::stringstream os;
		os << "set title '" << escape_quotes(title) << "'";
		return sendcommand(os);
	}

	bool set_xlabel(const std::string& label) {
		std::stringstream os;
		os << "set xlabel '" << escape_quotes(label) << "'";
		return sendcommand(os);
	}

	bool set_ylabel(const std::string& label) {
		std::stringstream os;
		os << "set ylabel '" << escape_quotes(label) << "'";
		return sendcommand(os);
	}

	bool set_zlabel(const std::string& label) {
		std::stringstream os;
		os << "set zlabel '" << escape_quotes(label) << "'";
		return sendcommand(os);
	}

	void set_xrange(double min = NAN, double max = NAN) {
		xrange = format_range(min, max);
	}

	void set_yrange(double min = NAN, double max = NAN) {
		yrange = format_range(min, max);
	}

	void set_zrange(double min = NAN, double max = NAN) {
		zrange = format_range(min, max);
	}

	bool set_grid() {
		std::stringstream os;
		os << "set grid";
		return sendcommand(os);
	}

	bool set_logscale(AxisScale scale) {
		switch (scale) {
		case AxisScale::LOGX:
			return sendcommand("set logscale x");
		case AxisScale::LOGY:
			return sendcommand("set logscale y");
		case AxisScale::LOGXY:
			return sendcommand("set logscale xy");
		default:
			return sendcommand("unset logscale");
		}
	}

#pragma endregion PLOT PROPERTIES SETTERS

#pragma region PLOT METHODS

	template <typename T>
	void plot(const std::vector<T>& y, const std::string& label = "",
		LineStyle style = LineStyle::LINES) {
		_plot(label, style, false, y);
	}

	template <typename T, typename U>
	void plot(const std::vector<T>& x, const std::vector<U>& y,
		const std::string& label = "", LineStyle style = LineStyle::LINES) {
		_plot(label, style, false, x, y);
	}

	template <typename T, typename U, typename V>
	void plot_xerr(const std::vector<T>& x, const std::vector<U>& y,
		const std::vector<V>& err, const std::string& label = "") {
		_plot(label, LineStyle::X_ERROR_BARS, false, x, y, err);
	}

	template <typename T, typename U, typename V>
	void plot_yerr(const std::vector<T>& x, const std::vector<U>& y,
		const std::vector<V>& err, const std::string& label = "") {
		_plot(label, LineStyle::Y_ERROR_BARS, false, x, y, err);
	}

	template <typename T, typename U, typename V, typename W>
	void plot_xyerr(const std::vector<T>& x, const std::vector<U>& y,
		const std::vector<V>& xerr, const std::vector<W>& yerr,
		const std::string& label = "") {
		_plot(label, LineStyle::XY_ERROR_BARS, false, x, y, xerr, yerr);
	}

	template <typename T, typename U, typename V, typename W>
	void plot_vectors(const std::vector<T>& x, const std::vector<U>& y,
		const std::vector<V>& vx, const std::vector<W>& vy,
		const std::string& label = "") {
		_plot(label, LineStyle::VECTORS, false, x, y, vx, vy);
	}

	void plot3d(const std::string& func, const std::string& label = "", bool use_color_gradient = false) {
		is_3dplotfunc = true;
		series3d.push_back(Gnuplot3dFunctionSeries{ func, !use_color_gradient, label });
	}

	void plot_dot(const std::vector<double>& point, const std::string& label = "") {
		has_dot = true;
		dots.push_back(GnuplotDots{ point, label });
	}

	template <typename T, typename U>
	void plot3d(const std::vector<T>& x, const std::vector<U>& y,
		const std::vector<U>& z, const std::string& label = "",
		LineStyle style = LineStyle::LINES) {
		_plot(label, style, true, x, y, z);
	}

	template <typename T, typename U, typename V, typename W>
	void plot_vectors3d(const std::vector<T>& x, const std::vector<U>& y,
		const std::vector<T>& z, const std::vector<V>& vx,
		const std::vector<W>& vy, const std::vector<T>& vz,
		const std::string& label = "") {
		_plot(label, LineStyle::VECTORS, true, x, y, z, vx, vy, vz);
	}

	/* Add a point and a X error bar to the list of samples to be plotted */
	void add_point_xerr(double x, double y, double err) {
		check_consistency();

		list_of_x.push_back(x);
		list_of_y.push_back(y);
		list_of_xerr.push_back(err);
	}

	/* Add a point and a Y error bar to the list of samples to be plotted */
	void add_point_yerr(double x, double y, double err) {
		check_consistency();

		list_of_x.push_back(x);
		list_of_y.push_back(y);
		list_of_yerr.push_back(err);
	}

	/* Add a point and two X/Y error bars to the list of samples to be plotted */
	void add_point_xyerr(double x, double y, double xerr, double yerr) {
		check_consistency();

		list_of_x.push_back(x);
		list_of_y.push_back(y);
		list_of_xerr.push_back(xerr);
		list_of_yerr.push_back(yerr);
	}
	/* Add a point to the list of samples to be plotted */
	void add_point(double x, double y) {
		check_consistency();

		list_of_x.push_back(x);
		list_of_y.push_back(y);
	}

	/* Add a value to the list of samples to be plotted */
	void add_point(double y) {
		add_point(static_cast<double>(list_of_x.size()), y);
	}

	/* Return the number of points added by `add_point` */
	int get_num_of_points() const {
		check_consistency();

		return (int)list_of_x.size();
	}

	/* Return the list of abscissas for the points added by `add_point` */
	const std::vector<double>& get_points_x() const { return list_of_x; }

	/* Return the list of ordinates for the points added by `add_point` */
	const std::vector<double>& get_points_y() const { return list_of_y; }

	/* Create a plot using the values set with the method `add_point` */
	void plot(const std::string& label = "", LineStyle style = LineStyle::LINES) {
		check_consistency();

		_plot(label, style, false, list_of_x, list_of_y);
	}

	/* Create a plot with X error bars using the values set with the method `add_point` */
	void plot_xerr(const std::string& label = "") {
		check_consistency();

		plot_xerr(list_of_x, list_of_y, list_of_xerr, label);
	}

	/* Create a plot with X error bars using the values set with the method `add_point` */
	void plot_yerr(const std::string& label = "") {
		check_consistency();

		plot_yerr(list_of_x, list_of_y, list_of_yerr, label);
	}

	/* Create a plot with X error bars using the values set with the method `add_point` */
	void plot_xyerr(const std::string& label = "") {
		check_consistency();

		plot_xyerr(list_of_x, list_of_y, list_of_xerr, list_of_yerr, label);
	}

	template <typename T>
	void histogram(const std::vector<T>& values, size_t nbins,
		const std::string& label = "",
		LineStyle style = LineStyle::BOXES) {
		assert(nbins > 0);

		if (values.empty())
			return;

		if (!series.empty()) {
			assert(!is_3dplot);
		}

		double min = *std::min_element(values.begin(), values.end());
		double max = *std::max_element(values.begin(), values.end());
		double binwidth = (max - min) / nbins;

		std::vector<size_t> bins(nbins);
		for (const auto& val : values) {
			int index = static_cast<int>((val - min) / binwidth);
			if (index >= int(nbins))
				--index;

			bins.at(index)++;
		}

		std::stringstream of;
		for (size_t i{}; i < nbins; ++i) {
			of << min + binwidth * (i + 0.5) << " " << bins[i] << "\n";
		}

		series.push_back(GnuplotSeries{ of.str(), style, label, "1:2" });
		is_3dplot = false;
	}

#pragma endregion PLOT METHODS

	// Ask Gnuplot to use a multiple-plot layout
	bool multiplot(int nrows, int ncols, const std::string& title = "") {
		std::stringstream os;
		os << "set multiplot layout " << nrows << ", " << ncols << " title '"
			<< escape_quotes(title) << "'\n";
		return sendcommand(os);
	}

	// Force Gnuplot to draw all the series sent through any of the `plot`
	// commands
	bool show(bool call_reset = true) {
		useAutoshow = false;
		bool result = false;

		/*if (series.empty())
			return true;*/

		std::stringstream os;
		os << "set style fill transparent solid 0.5\n";
		//os << "set pm3d noborder";


		// Write the data in separate series
		for (size_t i{}; i < series.size(); ++i) {
			const GnuplotSeries& s = series.at(i);
			os << "$Datablock" << i << " << EOD\n" << s.data_string << "\nEOD\n";
		}

		// код для построения 3D графика по функции
		if (is_3dplotfunc) {

			std::stringstream fs3d;
			fs3d << "set xrange " << xrange << "\n"
				<< "set yrange " << yrange << "\n"
				<< "set zrange " << zrange << "\n";

			sendcommand(fs3d);

			// isosample всегда задаются, если мы строим не плоскости, то добавляется "with pm3d", 
			// тем самым график становится разноцветным. Если плоскость, то используются isosample
			
			for (int i = 0; i < series3d.size(); i++) {
				const Gnuplot3dFunctionSeries& s = series3d.at(i);
				os3d << "\nf" << i + 1 << "(x,y)=" << s.function_string;
			}
			
			os3d << "\nset samples 100";
			os3d << "\nset isosamples 301";
			os3d << "\nset hidden3d";
			os3d << "\nset parametric";
			os3d << "\nset urange " << xrange;
			os3d << "\nset vrange " << yrange;
			
			os3d << "\nsplot ";

			for (size_t i = 0; i < series3d.size(); ++i) {

				const Gnuplot3dFunctionSeries& s = series3d.at(i);

				os3d << "u, v, f" << i + 1 << "(u,v)" << " title '" << s.title << "'";


				if (!s.is_plane) {
					os3d << " with pm3d";
				}

				if (i + 1 < series3d.size()/* || has_dot*/) {
					os3d << ", \\\n";
				}
			}

			// если еще передали точки, то добавляем их на график в ручном режиме
			if (has_dot) {
				for (const auto& point : dots) {
					os3d << ", '-'  with points pt 7 lc rgb 'red' title '" << point.title << "'\n" << point.coordinates[0] << " " << point.coordinates[1] << " " << point.coordinates[2];
				}
				os3d << "\ne";
			}
			sendcommand(os3d.str());

		} else { // else отрабатывает, если мы строим функцию по координатам

			if (is_3dplot) {
				os << "set hidden3d\n";
				os << "set dgrid3d 40,40\nset pm3d \n";
				os << "splot " << xrange << " " << yrange << " " << zrange << " ";
			} else {
				os << "plot " << xrange << " " << yrange << " ";
			}

			// Plot the series we have just defined
			for (size_t i{}; i < series.size(); ++i) {
				const GnuplotSeries& s = series.at(i);
				os << "$Datablock" << i << " using " << s.column_range << " with "
					<< style_to_str(s.line_style) << " title '" << escape_quotes(s.title)
					<< "'";

				if (i + 1 < series.size())
					os << ", ";
			}

			result = sendcommand(os);
		}

		if (result && call_reset)
			reset();

		return result;
	}

	// Remove all the series from memory and start with a blank plot
	void reset() {
		series.clear();
		set_xrange();
		set_yrange();
		is_3dplot = false;
	}

private:
	void _print_ith_elements(std::ostream&, std::ostream&, int, size_t) {}

	template <typename T, typename... Args>
	void _print_ith_elements(std::ostream& os, std::ostream& fmts, int index,
		size_t i, const std::vector<T>& v, Args... args) {
		os << v[i] << " ";

		if (i == 0) {
			if (index > 1)
				fmts << ':';
			fmts << index;
		}

		_print_ith_elements(os, fmts, index + 1, i, args...);
	}

	template <typename T, typename... Args>
	void _plot(const std::string& label, LineStyle style, bool is_this_3dplot,
		const std::vector<T>& v, Args... args) {
		if (v.empty())
			return;

		if (!series.empty()) {
			// Check that we are not adding a 3D plot to a 2D plot, or vice versa
			assert(is_3dplot == is_this_3dplot);
		}

		std::stringstream of;
		std::stringstream fmtstring;
		for (size_t i{}; i < v.size(); ++i) {
			_print_ith_elements(of, fmtstring, 1, i, v, args...);
			of << "\n";
		}

		series.push_back(GnuplotSeries{ of.str(), style, label, fmtstring.str() });
		is_3dplot = is_this_3dplot;
	}

	struct GnuplotSeries {
		std::string data_string;
		LineStyle line_style;
		std::string title;
		std::string column_range;
	};

	struct Gnuplot3dFunctionSeries {
		std::string function_string;
		bool is_plane;
		std::string title;
	};

	struct GnuplotDots {
		std::vector<double> coordinates;
		std::string title;
	};


	std::string style_to_str(LineStyle style) {
		switch (style) {
		case LineStyle::DOTS:
			return "dots";
		case LineStyle::POINTS:
			return "points";
		case LineStyle::LINESPOINTS:
			return "linespoints";
		case LineStyle::STEPS:
			return "steps";
		case LineStyle::BOXES:
			return "boxes";
		case LineStyle::X_ERROR_BARS:
			return "xerrorbars";
		case LineStyle::Y_ERROR_BARS:
			return "yerrorbars";
		case LineStyle::XY_ERROR_BARS:
			return "xyerrorbars";
		case LineStyle::VECTORS:
			return "vectors";
		default:
			return "lines";
		}
	}

	std::string format_range(double min = NAN, double max = NAN) {
		if (std::isnan(min) && std::isnan(max)) return "[]";

		std::stringstream os;
		os << "[";

		if (std::isnan(min)) os << "*";
		else os << min;

		os << ":";
		if (std::isnan(max)) os << "*";
		else os << max;

		os << "]";

		return os.str();
	}

	FILE* connection;
	std::vector<GnuplotSeries> series;
	std::vector<Gnuplot3dFunctionSeries> series3d;
	std::vector<GnuplotDots> dots;
	std::vector<std::string> files_to_delete;
	std::stringstream os3d;
	std::string xrange;
	std::string yrange;
	std::string zrange;
	bool is_3dplot;
	bool has_dot;
	bool is_3dplotfunc = false;
};