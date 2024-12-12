#include <iostream>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <functional>
#include <chrono>
using namespace std;

int matrixSize = 0;               //rozmiar rozpatrywanego grafu
int bestLen = 0;  //koszt najlepszego znalezionego rozwiązania
vector<int> bestSolution;   //najlepsze znalezione rozwiązanie
int coolingType = 0;        //typ chlodzenia
vector<int> randomNumbers;  //vektor liczb pseudolosowych
int randomNumbersIndex = 0; //indeks do losowania kolejnych liczb
int randomNumbersLen = 0; //dlugosc wektora liczb losowych -1
vector<int> currentSolution;// obecnie rozpatrywane rozwiązanie
int currentLen = 0;         //koszt obecnie rozpatrywanego rozwiązania
double currentTemperature;  //aktualna temperatura

chrono::high_resolution_clock::time_point startTime = chrono::high_resolution_clock::now(); //czas rozpoczęcia programu
chrono::high_resolution_clock::time_point stopTime = chrono::high_resolution_clock::now(); //czas rozpoczęcia programu
// funkcje zmieniająca temperature
function<double(double, int, double)> calcNewTemperature;


//podawanie liczb pseudolosowych ze wczytanej wcześniej listy
inline double getRandomNumber() {
    double result = double(randomNumbers[randomNumbersIndex]) / 2147483647;

    if (randomNumbersIndex >= randomNumbersLen) {
        randomNumbersIndex = 0;
    }else{
        randomNumbersIndex += 1;
    }
    return result;
}


//algorytm chciwy
void greedyAlg(int **matrix) {
    int currentCity = 0;
    bestSolution.push_back(currentCity);
    bestLen = 0;
    int nextCity = 0;
    int minLen = 0;
    //dopóki ścieżka nie zawiera wszystkich wierzchołków grafu, pętla trwa
    while (bestSolution.size() != matrixSize) {
        minLen = 2147483647;
        //przeglądamy wszystkie krawędzie wychodzące z pierwszego wierzchołka
        for (int i = 0; i < matrixSize; i++) {
            //sprawdzamy czy dany wierzchołek był już odwiedzony
            auto it = find(bestSolution.begin(), bestSolution.end(), i);
            //jeśli był, idziemy do kolejnego
            if (it != bestSolution.end()) continue;
                //jeśli niema go na liście sprawdzamy czy krawędź do niego jest mniejsza niż najmniejsza znalezniona
            else {
                if (matrix[currentCity][i] < minLen) {
                    nextCity = i;
                    minLen = matrix[currentCity][nextCity];
                }
            }
        }
        bestSolution.push_back(nextCity);
        bestLen += minLen;
        currentCity = nextCity;
    }
    //na końcu dodajemy powrót do początkowego wierzchołka aby powstał cykl
    bestSolution.push_back(0);
    //zwiększamy również odpowiednio długość znalezionego cyklu
    bestLen += matrix[currentCity][0];
}


//funkcja zamienia punkty miejscami
inline void swapPoints(int i, int j, vector<int> &testSolution) {

    swap((testSolution)[i], (testSolution)[j]);

    //jeśli któryś z pynktów ma indeks 0
    // musimy zwrócić uwage aby również ostatni wierzchołek cyklu zamienić
    if (i == 0 or j == 0) {
        testSolution[matrixSize] = testSolution[0];
    }
}


//wyliczamy różnice kosztów prze i po operacją swap
//funkcja powinna byc wykonywana przed operacją swap
inline int calcDiff(int **matrix, vector<int> &testSolution, int indexI, int indexJ) {
    int diff = 0;
    if (indexI == 0 or indexJ == 0) {
        if (max(indexI, indexJ) == 1) {
            diff -= matrix[testSolution[0]][testSolution[1]];
            diff -= matrix[testSolution[1]][testSolution[2]];
            diff -= matrix[testSolution[matrixSize - 1]][testSolution[matrixSize]];

            diff += matrix[testSolution[1]][testSolution[0]];
            diff += matrix[testSolution[0]][testSolution[2]];
            diff += matrix[testSolution[matrixSize - 1]][testSolution[1]];
        } else if (max(indexI, indexJ) == matrixSize - 1) {
            diff -= matrix[testSolution[0]][testSolution[1]];
            diff -= matrix[testSolution[matrixSize - 2]][testSolution[matrixSize - 1]];
            diff -= matrix[testSolution[matrixSize - 1]][testSolution[matrixSize]];

            diff += matrix[testSolution[matrixSize - 1]][testSolution[1]];
            diff += matrix[testSolution[matrixSize - 2]][testSolution[0]];
            diff += matrix[testSolution[0]][testSolution[matrixSize - 1]];
        } else {
            int index = max(indexI, indexJ);
            diff -= matrix[testSolution[0]][testSolution[1]];
            diff -= matrix[testSolution[index - 1]][testSolution[index]];
            diff -= matrix[testSolution[index]][testSolution[index + 1]];
            diff -= matrix[testSolution[matrixSize - 1]][testSolution[matrixSize]];

            diff += matrix[testSolution[index]][testSolution[1]];
            diff += matrix[testSolution[index - 1]][testSolution[0]];
            diff += matrix[testSolution[0]][testSolution[index + 1]];
            diff += matrix[testSolution[matrixSize - 1]][testSolution[index]];
        }

    } else if (indexI + 1 == indexJ or indexJ + 1 == indexI) {
        int index1 = min(indexI, indexJ);
        int index2 = max(indexI, indexJ);

        diff -= matrix[testSolution[index1 - 1]][testSolution[index1]];//todo opisac
        diff -= matrix[testSolution[index1]][testSolution[index2]];
        diff -= matrix[testSolution[index2]][testSolution[index2 + 1]];

        diff += matrix[testSolution[index1 - 1]][testSolution[index2]];
        diff += matrix[testSolution[index2]][testSolution[index1]];
        diff += matrix[testSolution[index1]][testSolution[index2 + 1]];
    } else {
        diff -= matrix[testSolution[indexJ - 1]][testSolution[indexJ]];
        diff -= matrix[testSolution[indexJ]][testSolution[indexJ + 1]];
        diff -= matrix[testSolution[indexI - 1]][testSolution[indexI]];
        diff -= matrix[testSolution[indexI]][testSolution[indexI + 1]];

        diff += matrix[testSolution[indexI - 1]][testSolution[indexJ]];
        diff += matrix[testSolution[indexJ]][testSolution[indexI + 1]];
        diff += matrix[testSolution[indexJ - 1]][testSolution[indexI]];
        diff += matrix[testSolution[indexI]][testSolution[indexJ + 1]];

    }
    return diff;
}


double calcBeginTemperature(int **matrix, vector<int> &testSolution) {
    testSolution = bestSolution;
    int testLen = 0;
    int minLen = 2147483647;
    int maxLen = 0;
    int indexI = 0;
    int indexJ = 0;
//dla stu cykli znajdujemy najmniejszą i największą wagę. Ich różnicę pomnożoną razy pewien współczynnik zwracamy jako temperature początkową
    for (int i = 0; i < 100; i++) {
        indexI = (int) (getRandomNumber() * matrixSize);
        indexJ = indexI;
        while (indexI == indexJ) {
            indexJ = (int) (getRandomNumber() * matrixSize);
        }
        testLen += calcDiff(matrix, testSolution, indexI, indexJ);
        swapPoints(indexI, indexJ, testSolution);

        minLen = min(minLen, testLen);
        maxLen = max(maxLen, testLen);
    }
    return (maxLen - minLen) * 1.5;
}



void start(int **matrix, double coolingFactor, int expectedLen) {
    vector<int> testSolution;
    int indexI = 0;
    int indexJ = 0;
    //na początku algorytmem chciwym wybierane jest początkowe rozwiązanie
    greedyAlg(matrix);    //początkowa temperatura jest iloczynem kosztu najlepszego znalezionego rozwiązania oraz pewnego wspolczynnika
    double beginningTemperature = calcBeginTemperature(matrix, testSolution);

    //długość epoki jest zależna od wielkości problemu
    int epochLen = matrixSize * (matrixSize - 1) / 2;


    //inicjalizujemy generator liczb losowych w przedziale 0-1
    currentSolution = bestSolution;
    currentLen = bestLen;


    //na potrzeby dokladnego zbadania problemu zainicjalizowane zmienne opisujące 3 rozwiązania.
    //najlepsze znalezione rozwiązanie - best*,
    //rozwiązanie którego sąsiadów badamy - current* oraz rozwiązanie którego akceptacje rozważamy test*
    currentTemperature = beginningTemperature;
    int iteration = 0;

    int delta = 0;

    int testLen = bestLen;
    testSolution = currentSolution;
    int testLenBefore = 0;
    while (currentTemperature >= 0.001 and bestLen >= expectedLen) {
        //w każdej epoce jest jednakowa ilość iteracji przy jednakowej temperaturze
        for (int epoch = 0; epoch < epochLen; epoch++) {
            //losujemy punkty których zamiana określi nam kolejnego sąsiada
            indexI = (int) (getRandomNumber() * matrixSize);
            indexJ = indexI;
            while (indexI == indexJ) {
                indexJ = (int) (getRandomNumber() * matrixSize);
            }

            testLenBefore = testLen;
            //oraz liczymy jego koszt
            testLen += calcDiff(matrix, testSolution, indexI, indexJ);

            //obliczamy różnice kosztów sąsiada i obecnego rozwiązania
            delta = testLen - currentLen;

            //jeśli sąsiad ma mniejszy koszt, przechodzimy do niego
            if (delta <= 0) {
                //przechodzimy do nowego sąsiada
                swapPoints(indexI, indexJ, testSolution);
                currentLen = testLen;
                swapPoints(indexI, indexJ, currentSolution);
                //sprawdzamy czy obecne rozwiązanie nie jest najlepszym dotychczas znalezionym
                if (currentLen < bestLen) {
                    bestLen = currentLen;
                    bestSolution = currentSolution;
                }
            } else if (getRandomNumber() < exp(-delta / currentTemperature)) {
                //przechodzimy do nowego sąsiada
                swapPoints(indexI, indexJ, testSolution);

                //jeśli sąsiad ma większy koszt niż nasze rozwiązanie i tak ma szansę być nowym rozwiązaniem
                //ze wzoru sprawdzamy czy koszt (energia) mieści się w akceptowanych wylosowanych przez nas granicach
                currentLen = testLen;
                swapPoints(indexI, indexJ, currentSolution);

            } else {
                testLen = testLenBefore;
            }
        }
        //inkrementujemy nimer epoki oraz wyliczamy nową temperaturę
        iteration++;

        currentTemperature = calcNewTemperature(currentTemperature, iteration, coolingFactor);
        //Jeśli temperatura przekroczyła pewien poziom zatrzymujemy algorytm gdyż szanse, że przyniesie nam
        // lepsze rozwiązanie zmniejszają sie wraz z temperaturą
        //drógim warunkiem jest przekroczenie preferowanego przez nas czasu działania algorytmu
    }
}

//usuwanie macierzy sąsiedstwa z pamięci
void clearAll(int **matrix) {
    if (matrixSize != 0) {
        for (int i = 0; i < matrixSize; i++) {
            delete[] matrix[i];
        }
        matrixSize = 0;
    }
    delete[] matrix;

    bestSolution.clear();
}


//wczytanie macierzy sąsiedstwa z pliku
int **readMatrix(const string &path, int **matrix) {
    if (matrixSize != 0) clearAll(matrix);   //usunięcie poprzedniego grafu

    int size = 0;
    //otwarcie pliku
    ifstream file(path);

    //sprawdzenie, czy udało się otworzyć plik
    if (!file.is_open()) {
        return nullptr;
    }

    //zmienna przechowująca wartość wczytaną z pliku
    string word;

    //wyczyszczenie flagi błędów i pozycję odczytu pliku
    file.clear();
    file.seekg(0, ios::beg);    //wskaznik na początku pliku (beg) przesunięty o 0 znaków



    //wczytanie rozmiaru grafu po znalezieniu słowa "DIMENSION"
    while (file >> word) {
        if (word.find("DIMENSION:") != string::npos) {
            file >> size;
            //istringstream iss(word);
            matrix = new int *[size];   //deklaracja nowych rozmiarów macierzy
            for (int i = 0; i < size; i++) {
                matrix[i] = new int[size];
            }
            matrixSize = size;
            break;
        }
    }

    // szukanie ciągu znaków "EDGE_WEIGHT_SECTION"
    while (file >> word) {
        if (word.find("EDGE_WEIGHT_SECTION") != string::npos) {
            break;
        }
    }

    //wczytanie macierzy sąsiedstwa grafu
    int x = 0;
    int y = 0;
    while (file >> word) {
        istringstream iss(word);
        int weight = 0;
        while (iss >> weight) {     //wczytywanie wag
            matrix[y][x] = weight;
            x++;
            if (x == size) {
                x = 0;
                y++;
            }
        }
    }
    file.close();   //zamykanie pliku
    return matrix;    //metoda zwraca true jeśli wszystko się udało wczytać
}


bool readRandomNumbers(string path) {
    //otwarcie pliku
    ifstream file(path);

    //sprawdzenie, czy udało się otworzyć plik
    if (!file.is_open()) {
        return false;
    }

    //zmienna przechowująca wartość wczytaną z pliku
    string line;

    //wyczyszczenie flagi błędów i pozycję odczytu pliku
    file.clear();
    file.seekg(0, ios::beg);    //wskaznik na początku pliku (beg) przesunięty o 0 znaków

    while (file >> line) {
        randomNumbers.push_back(stoi(line));
    }
    randomNumbersIndex = 0;
    randomNumbersLen = randomNumbers.size() - 1;
    file.close();   //zamykanie pliku
    return true;    //metoda zwraca true jeśli wszystko się udało wczytać
}






int main() {
    SetConsoleOutputCP(CP_UTF8);    //ustawianie polskich znaków

    cout << "Autor: Kasper Radom 264023" << endl << endl;





    cout << "Podaj ścieżkę do pliku konfiguracyjnego" << endl << ">>";//todo odkomwntować
    string path;
    cin >> path;
    cout << endl;
//    path = R"(C:\Users\radom\OneDrive\Pulpit\SimulatedAnnealing_JuliaVSCpp\config.ini)";

    string atspPath;                //ścieżka do pliku z reprezentacją grafu
    int expectedLen;                //koszt oczekiwanego rozwiązania
    double coolingFactor;  //wspolczynnik chlodzenia


    //wczytywanie pliku
    ifstream file(path);

    if (!file.is_open()) {
        cerr << "Nie można otworzyć pliku!" << endl;
        return 1;
    }

    //wyczyszczenie flagi błędów i pozycję odczytu pliku
    file.clear();
    file.seekg(0, ios::beg);    //wskaznik na początku pliku (beg) przesunięty o 0 znaków

    //zmienna przechowująca wartość wczytaną z pliku
    string line;

    //Wczytywanie liczb pseudolosowych
    getline(file, line);

    if (!readRandomNumbers(line)) {
        cerr << "Nie można otworzyć pliku z zestawem liczb pseudolosowych!" << endl;
        return 1;
    }

    //wczytywanie kolejnych linii
    while (getline(file, line)) {
        //pominięcie zakomentowanych wierszy
        if (line[0] == '#' or line.length() == 0){
            continue;
        }
        cout<<line<<endl<<endl; //wypisywanie postępu algorytmu

        istringstream iss(line);  // tworzymy strumień wejściowy z tekstu

        // Wczytujemy ścieżkę do pliku ATSP
        if (!(iss >> atspPath)) {
            cerr << "Błąd przy wczytywaniu ścieżki do pliku z reprezentacją grafu" << endl;
            continue;
        }


        // Wczytujemy kryterium stopu
        if (!(iss >> expectedLen)) {
            cerr << "Błąd przy wczytywaniu expectedLen" << endl;
            continue;
        }
        //oczekiwane rozwiązanie jest akceptowane gdy jest co najwyżej o 1% dorsze od oczekiwanego
        expectedLen = expectedLen*1.01;
        // Wczytujemy współczynnik zmiany temperatury
        if (!(iss >> coolingFactor)) {
            cerr << "Błąd przy wczytywaniu coolingFactor" << endl;
            continue;
        }

        // Wczytujemy typ schładzania
        if (!(iss >> coolingType)) {
            cerr << "Błąd przy wczytywaniu coolingType" << endl;
            continue;
        }

        // Wczytujemy typ schładzania
        if (!(iss >> randomNumbersIndex)) {
            cerr << "Błąd przy wczytywaniu randomNumbersIndex" << endl;
            continue;
        }
        if(randomNumbersIndex >= randomNumbers.size()) randomNumbersIndex = 0;

        //metoda obliczająca nową temperaturę w zależności od wybranego typu chłlodzenia
        switch (coolingType) {
            //chłodzenie geometryczne
            case 1:
                calcNewTemperature = [](double T, int iteration, double coolingFactor) -> double {
                    return T * coolingFactor;
                };
                break;

                //chłodzenie logarytmiczne
            case 2:
                calcNewTemperature = [](double T, int iteration, double coolingFactor) -> double {
                    return T / (1 + coolingFactor * log(iteration + 1));
                };
                break;

                //chłodzenie ekspotencjalne
            default:
                calcNewTemperature = [](double T, int iteration, double coolingFactor) -> double {
                    return T * exp(-coolingFactor * iteration);
                };
                break;
        }

        int **matrix{};
        matrix = readMatrix(atspPath, matrix);

        // mierzenie czasu przed wykonaniem funkcji
        startTime = chrono::high_resolution_clock::now();

        //uruchomienie algorytmu
        start(matrix, coolingFactor, expectedLen);

        // mierzenie pamięci i czasu po wykonaniu funkcji
        stopTime = chrono::high_resolution_clock::now();


        clearAll(matrix);



        string fileName = "wynikiCPP.csv";

        //Zapis wyników na bierząco do pliku
        // Otwieranie pliku w trybie dopisywania (ios::app)
        ofstream resultFile(fileName, ios::app);

        // Sprawdzenie, czy plik został poprawnie otwarty
        if (!resultFile.is_open()) {
            cerr << "Nie można otworzyć pliku: " << fileName << endl;
        }

        // Zapis wyników
        resultFile  << bestLen<<";"
                    << chrono::duration_cast<chrono::duration<double>>(stopTime - startTime).count()
                    <<'\n';

        // Zamknięcie pliku
        resultFile.close();

    }

    file.close();  // Zamykamy plik

}






