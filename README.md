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

```cpp
#include "gnuplot.hpp"

double f(double x) {
	return 19 * x * x + 84 * sin(x) - 18;
}

int main() {

	Gnuplot plt{ R"("C:\Program Files\gnuplot\bin\gnuplot.exe")" };

	const double a = -3, b = 1, step = 0.01;

	std::vector<double> x, y;

	for (double i = a; i <= b; i += step) {
		x.push_back(i);
		y.push_back(f(i));
	}

	plt.plot(x, y, "19 x^2 + 84 sin(x) - 18 = 0");
	plt.set_title("График функции");
	plt.set_grid();
	plt.set_xlabel("x");
	plt.set_ylabel("y");
	plt.show();

	return 0;
}
```

![](https://github.com/user-attachments/assets/672e560a-7b00-47e5-862a-876d1d205d5f)