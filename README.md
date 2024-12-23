# gnuplot-hpp

Заголовочный файл, реализующий интерфейс для изображения графиков с помощью `gnuplot` в консольном приложении `cpp`.

## Установка

### Windows

Загрузите gnuplot по [ссылке](https://sourceforge.net/projects/gnuplot/files/gnuplot/). Нажмите на кнопку "Download Latest Version" и скачайте файл с расширением `.exe`.

Запустите установщик и в ходе конфигурации установки выберите флажок *Add application directory to your PATH environment variable* для автоматического добавления пути к gnuplot.exe в переменную среды PATH.

### Linux

Ввести в менеджере пакетов следующие команды для установки gnuplot:

    # Debian, Ubuntu, Linux Mint
    sudo apt-get install gnuplot
    
    # Arch, Manjaro
    sudo pacman -Syu gnuplot
    
    # Fedora
    sudo dns install gnuplot

### Mac OS X

Предпочтительным способом установки gnuplot является использование [Homebrew](https://formulae.brew.sh/formula/gnuplot) из терминала:

    brew install gnuplot

### Установка `gnuplot.hpp`

Скачайте файл [gnuplot.hpp](https://github.com/LeoKhariton/gnuplot-hpp/blob/master/gnuplot.hpp) из этого репозитория и сохраните его в каталоге с проектом.

## Примеры

### Изображение графика функции в декартовых координатах

```cpp
#include "gnuplot.hpp"
#include <cmath>
#include <vector>

typedef std::vector<double> vector;

double poly(const vector& coeffs, double x) {
	double result = 0;
	int n = coeffs.size();
	for (int i = 0; i < n; i++)
		result += coeffs[i] * pow(x, n - 1 - i);
	return result;
}

int main() {
	//Gnuplot plt{ };
	Gnuplot plt{ R"("C:\Program Files\gnuplot\bin\gnuplot.exe")" };

	vector x = { 0.75, 1.5, 2.25, 3, 3.75 };
	vector y = { 2.5, 1.2, 3.1, 2.25, 4.48 };

	vector coeffs = { 1.551, -13.99, 43.96, -55.4, 24.73 };

	std::vector<double> xs, ys;

	for (double i = x[0]; i <= x.back(); i += 0.01) {
		xs.push_back(i);
		ys.push_back(poly(coeffs, i));
	}

	plt.plot(x, y, "Исходные данные", Gnuplot::LineStyle::POINTS);
	plt.plot(xs, ys, "Полином Лагранжа");
	plt.set_title("График функции");
	plt.set_grid();
	plt.set_xlabel("x");
	plt.set_ylabel("y");

	return 0;
}
```

![](https://github.com/user-attachments/assets/672e560a-7b00-47e5-862a-876d1d205d5f)