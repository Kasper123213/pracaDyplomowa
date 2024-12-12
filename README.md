# Projekt: Porównanie Wydajności Julia vs C++

Ten projekt służy do porównania wydajności algorytmu symulowanego wyżarzania zaimplementowanego w językach Julia i C++ na przykładzie problemu komiwojażera (TSP).

## Wymagania

Aby uruchomić projekt, potrzebujesz:
- Julia (w wersji co najmniej 1.9)
- Kompilator C++ (np. g++)
- Python (opcjonalnie, jeśli chcesz generować wykresy)

## Instrukcja uruchomienia
- Skopiuj repozytorium na swój lokalny komputer:
  ```bash
  git clone https://github.com/Kasper123213/pracaDyplomowa
  cd pracaDyplomowa
  ```
- Wygeneruj listę liczb losowych:
  ```bash
  julia numbersGenerator.jl
  ```
- dostosuj zawartość pliku config.ini
- skompiluj plik main.cpp
  ```bash
  g++ -Ofast -o CPPvsJULIA.exe main.cpp -static -static-libstdc++ -static-libgcc -march=tigerlake
  ```
-uruchom program w julii i c++
```bash
start julia -O3 --check-bounds=no --inline=yes --math-mode=fast main.jl
start CPPvsJULIA.exe
```
