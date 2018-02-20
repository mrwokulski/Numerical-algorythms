/*
* Kompilowac za pomoca polecenia:
* g++ -I sciezka_do_eigena main.cpp -std=c++11 -03
*/
#include "stdafx.h"


using namespace Eigen;
using namespace std;

typedef SparseMatrix<double> SpMat; // declares a column-major sparse matrix type of double
typedef Triplet<double> T;

//ilosc grzybow na planszu 3 v 7 //ilosc scian kostki - koncowki indeksow - 4,0; l = 3 + 2(cyfraindeksu%4)
int N = 114; // N jest rozmiarem planszy, gdzie plansza sklada sie z N*2+1 pól
int sizeM = N * 2 + 1;
int k = 3; // 3 + 5%6 ; k to liczba grzybów na planszy
int s1 = -N; // s1 jest polem startowym gracza nr1
int s2 = N; // s2 jest polem startowym gracza nr2
int l = 3; // 3 + 2 * (4v0%4)
int rownania = (2 * N)*(2 * N) * 2;
double epsilon = 0.00000001; //do eigena
int iteracjeM = 10000;
int macierzM = 10000000; // maksymalny rozmiar macierzy
double gWynik;



//MIERZENIE CZASU
double PCFreq = 0.0;
__int64 CounterStart = 0;
void StartCounter()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
		printf("QueryPerformanceFrequency failed!\n");

	PCFreq = double(li.QuadPart) / 1000000.0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}

double GetCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}

// RAND + MOD Z ZAKRESEM

double fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}

int fRand(int max)
{

	int range = max * 2;
	int number = rand() % range - max;
	return number;
}

int mod(int a, int b) { return (a % b + b) % b; }


//pomocnicza do tworzenia macierzy
int indexOf(vector<int> p, int n)
{
	for (int i = 0; i < p.size(); i++) {
		if (p[i] == n) return i;
	}
	return -1;
}


struct P {
	int player, x, y, wsp, g1, g2;

	P() {};

	P(int p, int x_, int y_, int wsp_, int g_1, int g_2) {
		player = p;
		x = x_;
		y = y_;
		wsp = wsp_;
		g1 = g_1;
		g2 = g_2;
	}

	void print() {
		string result;

		result = "P" + to_string(player) + "(" + to_string(x) + ", " + to_string(y) + ")" + "[" + to_string(g1) + "," + to_string(g2) + "]" + "{";
		result += "}";
		result += "^" + to_string(wsp);

		cout << setw(40) << left << result;
	}

	string toString() {
		string result;
		result = to_string(player) + to_string(x) + to_string(y);

		for (int i = s1; i <= s2; i++) {
		}
		return result;
	}

	int status() {
		if (x == 0) {
			if (g1 > g2) {
				return 1; // 1 wygral
			}
			else if (g1 < g2) {
				return 0; // 2 wygral
			}
			else {
				return 1; // takie same 1 wygral
			}
		}
		else if (y == 0) {
			if (g1 > g2) {
				return 1; // 1 wygral
			}
			else if (g1 < g2) {
				return 0; // 2 wygral
			}
			else {
				return 0; // takie same 2 wygral
			}
		}
		else {
			return -1; // gra toczy sie dalej
		}
	}
};

//ALGORYTMY ROZWIAZYWANIA UKLADOW LINIOWYCH-----------------------------------------------------------

void GaussPartial(int l, vector<double> x, vector< vector<double> > a, vector<double> b) {
	printf("Gauss partial ");
	for (int i = 0; i < l; i++) { // znajdz pivot rzedu i zamien
		int maksimum = i; // najwiekszy index
		for (int j = i + 1; j < l; j++) {
			if (abs(a[j][i]) > abs(a[maksimum][i]))
				maksimum = j;
		}
		a[i].swap(a[maksimum]);
		swap(b[i], b[maksimum]); //pivotuj miedzy A - B
		for (int j = i + 1; j < l; j++) {
			double alpha = a[j][i] / a[i][i];
			b[j] = b[j] - (alpha * b[i]);
			for (int k = i; k < l; k++) {
				a[j][k] = a[j][k] - (alpha * a[i][k]);
			}
		}
	}
	for (int i = l - 1; i >= 0; i--) {
		double suma = 0;
		for (int j = i + 1; j < l; j++) {
			suma = suma + a[i][j] * x[j];
		}
		x[i] = (b[i] - suma) / a[i][i];
	}
	gWynik = x[0];
	printf("x[0] = %lf ", gWynik);
}

void GaussPartialSparseMatrix(int l, vector<double> x, vector< vector<double> > a, vector<double> b) {
	printf("Gauss partial dla macierzy rzadkiej ");
	for (int i = 0; i < l; i++) { // znajdz rzad pivota i zamien
		int maksimum = i; // najwiekszy index
		for (int j = i + 1; j < l; j++) {
			if (abs(a[j][i]) > abs(a[maksimum][i]))
				maksimum = j;
		}
		a[i].swap(a[maksimum]);
		swap(b[i], b[maksimum]); //pivotuj w A i B
		for (int j = i + 1; j < l; j++) {// wiersz to j
			if (a[j][i] == 0) continue; // jesli element 0 to przechodzi dalej
			double pom = a[j][i] / a[i][i];
			b[j] = b[j] - (pom * b[i]);
			for (int k = i; k < l; k++) {
				a[j][k] = a[j][k] - (pom * a[i][k]);
			}
		}
	}
	for (int i = l - 1; i >= 0; i--) {
		double suma = 0;
		for (int j = i + 1; j < l; j++) {
			suma = suma + a[i][j] * x[j];
		}
		x[i] = (b[i] - suma) / a[i][i];
	}
	gWynik = x[0]; //global
	printf("x[0] = %lf ", gWynik);
}

void gaussSeidelMethod(int ilosc, vector<double> x, vector< vector<double> > a, vector<double> b) {
	printf("Gauss-Siedel ");
	vector<double> Xpoprz(ilosc, 0);
	double SumaW;
	for (int i = 1; i <= iteracjeM; i++) { // global
		for (int j = 0; j < ilosc; j++) Xpoprz[j] = x[j];
		for (int j = 0; j < ilosc; j++) {
			SumaW = 0;
			for (int k = 0; k <= j - 1; k++) SumaW += a[j][k] * x[k];
			for (int k = j + 1; k < ilosc; k++) SumaW += a[j][k] * Xpoprz[k];
			x[j] = (b[j] - SumaW) / a[j][j];
		}
		double rzadN;
		double maksN = 0; // norma sprawdzana
		for (int j = 0; j < ilosc; j++) {
			rzadN = fabs(x[j] - Xpoprz[j]);
			if (rzadN > maksN) maksN = rzadN;
		}
		if (maksN < epsilon) break; //global
	}
	gWynik = x[0];
	printf("x[0] = %lf ", gWynik);
}


void jacobiMethod(int ilosc, vector<double> x, vector< vector<double> > a, vector<double> b)
{
	printf("Jacobi ");
	double SumaW = 0;
	double SumaS = 0;
	vector<double> Xpoprz(ilosc, 0);
	for (int i = 1; i < iteracjeM; i++) { //nastepne iteracje
		for (int j = 0; j < ilosc; j++) { //nastepne wiersze
			for (int k = 0; k < ilosc; k++) Xpoprz[k] = x[k];
			SumaW = 0; SumaS = 0;
			for (int k = 0; k < ilosc; k++) if (k != j) SumaS += (a[j][k] * x[k]);
			SumaW = (b[j] - SumaS) / a[j][j];
			x[j] = SumaW;
		}
		double rzadN; double maksN = 0; // norma sprawdzana
		for (int j = 0; j < ilosc; j++) {
			rzadN = fabs(x[j] - Xpoprz[j]);
			if (rzadN > maksN) maksN = rzadN;
		}
		if (maksN < epsilon) break; //global
	}
	gWynik = x[0];
	printf("x[0] = %lf ", gWynik);
}

//--------------------------------------------------------------------------------------------------------------

//drukowanie macierzy do pliku

void matrixFile(vector< vector<double> > A, vector<double> B)
{
	ofstream matFile;
	matFile.open("matrix.txt");
	int n = A.size();

	for (int i = 0; i<n; i++)
	{
		for (int j = 0; j<n; j++) matFile << A[i][j] << "\t";
		matFile << "| " << B[i] << "\n";
	}

	matFile << endl;
	matFile.close();
}

//przygotowanie planszy - samych pól
void prepMap(vector<int> &p, int n) {
	int pola = -n;
	for (int i = 0; i < 2 * n + 1; i++)p[i] = pola++;
}

//rozstawienie grzybow na plansz

vector< vector<P> > prepProbMatrix() {
	int n = N;
	int w = sizeM;

	set<string> powtorzenie; // set do sprawdzania jakie prawdopodobienstwa juz obliczalismy
							 // mapa do mapowania odwiedzonych prawdopodobienstw, tak aby przy kolejnym spotkaniu skopiowac wartosci i wspolczynniki
	map<string, int> powtorzeniaMap;
	map<string, int>::iterator iter;
	pair<map<string, int>::iterator, bool> result;

	// kostka i plansza ; l = 3
	vector<int> plansza(w, 0);
	prepMap(plansza, n);
	vector<int> kostka(l, 0);
	kostka[0] = -1;
	kostka[1] = 0;
	kostka[2] = 1;

	// Wektory do przechowywania prawdopodobienstw
	vector<P> line(l + 1, P());
	// deklaracja macierzy wynikowej
	vector< vector<P> > final(macierzM, line);

	// wartosci dla pierwszego zestawu ruchow
	int player = 1;
	int x = -n;
	int y = n;
	int wsp = 0;

	//inicjowanie poczatkowej wartosci w mapie powtorzen
	final[0][0] = P(player, x, y, wsp, 0, 0);
	result = powtorzeniaMap.insert(pair<string, int>(final[0][0].toString(), wsp++));
	powtorzenie.insert(final[0][0].toString());
	//sprawdzanie czy x v y nie wychodza poza plansze - fcja pomocnicza
	x = indexOf(plansza, x);
	y = indexOf(plansza, y);

	// mapa pomocnicza grzybow - <pozycja_grzyba, czy_zebrany>
	player = player % 2 + 1;

	//generowanie 1 rownania prawdopodobienstw
	for (int i = 1; i < l + 1; i++) {
		//zmienne pomocnicze
		int X_ = x, Y_ = y;
		int g1_ = 0, g2_ = 0;
		// ustawianie odpowiednich parametrow w zaleznosci ktory gracz ma ruch
		if (player == 2)
		{
			// teraz wykonuje ruch gracz pierwszy, tylko wypisywane sa prawdopodobienstwa dla gracza 2
			X_ += kostka[i - 1]; // zmiana pozycji gracza zgodnie ze sciana kostki
			X_ = plansza[mod(X_, w)]; // teraz juz mamy odpowiednie pole, a nie indeks pola			
			// zakres od -N do N; (Y_ % zakres + zakres) % zakres; wpierw byly if-y a potem kolega doradzil taka funkcje
			Y_ = plansza[mod(Y_, w)];
		}
		else // player 1
		{
			// ruch gracza drugiego
			Y_ += kostka[i - 1]; // zmiana pozycji gracza zgodnie ze sciana kostki
			Y_ = plansza[mod(Y_, w)]; // teraz juz mamy odpowiednie pole, a nie indeks pola
			X_ = plansza[mod(X_, w)];
		}
		// dodanie prawdo do rownania
		final[0][i] = P(player, X_, Y_, wsp, g1_, g2_);
		// jezeli ktorys gracz dotarl na mete ustawiamy wsp na -1 zeby dalej nie musiec go rozpisywac
		if (X_ == 0 || Y_ == 0) {
			final[0][i].wsp = -1;
			wsp--; // bo pozniej wsp++
		}
		result = powtorzeniaMap.insert(pair<string, int>(final[0][i].toString(), wsp++));
	}

	bool done;
	int r = 0; // row
	int c = 1; // column start = 1 bo col[0] = 1
	int wsp_;
	int i;
	int g1, g2;

	for (i = 1; i < macierzM; i++) {
		//zczytywanie wartosci z obliczanego rownania
		x = final[r][c].x;
		y = final[r][c].y;
		player = final[r][c].player;
		wsp_ = final[r][c].wsp;
		g1 = final[r][c].g1;
		g2 = final[r][c].g2;

		if (r == i) break;
		// dzieki temu uzyskujemy wszyskie prawdopodobienstwa nie omijamy zadnego
		c = c%l + 1;
		if (c == 1) r++;
		// dodaje nowe rownanie
		final[i][0] = P(player, x, y, wsp_, g1, g2);
		// sprawdzam w mapie czy juz nie bylo liczone
		done = powtorzenie.count(final[i][0].toString());
		//jesli bylo to
		if (done == 1 || (final[i][0].x == 0 || final[i][0].y == 0)) {
			//bylo liczone, albo ktorys z graczy wygral wiec nie licze dalej
			i--;
			continue;
		}
		else powtorzenie.insert(final[i][0].toString());
		//sprawdzam czy nie wychodzi za plansze
		x = indexOf(plansza, x);
		y = indexOf(plansza, y);
		player = player % 2 + 1;

		for (int j = 1; j < l + 1; j++) {
			int X_ = x, Y_ = y;
			int g1_ = g1, g2_ = g2;
			//mapa pomocnicza dla grzybów przy rozpisywaniu nowych prawdo
			if (player == 2) {
				X_ += kostka[j - 1];
				X_ = plansza[mod(X_, w)];
				Y_ = plansza[mod(Y_, w)];
			}
			else {
				Y_ += kostka[j - 1];
				Y_ = plansza[mod(Y_, w)];
				X_ = plansza[mod(X_, w)];
			}
			// wstawiony player
			final[i][j] = P(player, X_, Y_, wsp, g1_, g2_);
			// jezeli dotarlem do konca ustawiam wsp na -1 (rola wartownika)
			if (X_ == 0 || Y_ == 0) {
				final[i][j].wsp = -1;
				continue;
			}

			result = powtorzeniaMap.insert(pair<string, int>(final[i][j].toString(), final[i][j].wsp));
			if (result.second == 0) {
				// trafilismy juz na te prawdopodobienstwo kopiujemy wspolczynnik
				iter = powtorzeniaMap.find(final[i][j].toString());
				final[i][j].wsp = iter->second;
				continue;
			}
			wsp++;
		}
	}

	rownania = i;
	printf("\nIle rownan = %d\n", rownania);

	return final;
}

void convertToMatrix(vector< vector<P> > final, vector< vector<double> > &macierz, vector<double> &b, vector<double> roz) {
	for (int i = 0; i < rownania; i++) { //global
		for (int j = 0; j < l + 1; j++) {
			if (i == final[i][j].wsp) macierz[i][final[i][j].wsp] = 1; // jesli wspolczynnik taki sam wierszy to przekatna i 1
			else if (final[i][j].status() == -1) macierz[i][final[i][j].wsp] = -roz[j - 1]; // gra jest kontunuowana
			else if (final[i][j].status() == 1) b[i] += roz[j - 1]; // wygnrana, dodana do wektora b
		}
	}
}

int main()
{
	cout << fixed << setprecision(16);

	// parametry funkcji drukujacej wyniki Eigena
	IOFormat CommaInitFmt(17, DontAlignCols, " ", " ", "", "", "", "");

	for (N; N < 115; N++) {
		//Przygotowanie gry---------------------------------------------------------WIKTOR
		s1 = -N;
		s2 = N;
		sizeM = N * 2 + 1;

		printf("\n----   Dla N = %d   ----\n", N);
		//int mushroomPos[ileShrooms];
		//jesli wiecej grzybow niz pol to rozklad jest 1/ilosc_pol wiec grzyb na kazde pole; max 1 grzyb na 1 pole
		
		// test prepShroom	
		//Koniec przygotowañ-------------------------------------------------------DONE


		//Monte test-----------------------------------------------------------------------ROBERT		
		//Monte koniec ------------------------------------------------------------------ DONE

		// GENEROWANIE PRAWDOPODOBIENSTW
		// prawdopodobienstwa kostki
		vector<double> rozklad_rowny(l, (1.0 / l));
		vector< vector<P> > test = prepProbMatrix(); //bez wyswietlania arg=>false

			// zamykamy dla takich samych stron

			// global wynik
			double czas;
			//Wyniki Eigen
			/*VectorXd EB(rownania);
			VectorXd EX(rownania);
			for (int i = 0; i < rownania; i++) { //wpisanie macierzy do eigena
				EB(i) = B[i];
				EX(i) = 0;
			}

			vector<T> coefficients;    // list of non-zeros coefficients
			SpMat AS(rownania, rownania);
			coefficients.reserve(rownania * rownania);
			for (int i = 0; i < rownania; i++) {
				for (int j = 0; j < rownania; j++) coefficients.push_back(T(i, j, A[i][j]));
			}
			AS.setFromTriplets(coefficients.begin(), coefficients.end());
			SparseLU<SparseMatrix<double, ColMajor>, COLAMDOrdering<int> > solver;
			// Compute the ordering permutation vector from the structural pattern of A
			solver.analyzePattern(AS);
			// Compute the numerical factorization
			solver.factorize(AS);
			//Use the factors to solve the linear system
			StartCounter();
			EX = solver.solve(EB);
			czas = GetCounter();
			printf("%-35s: x[0] = %lf  Czas: %lf\n", "SparseLU()", EX(0), czas);
			*/

			VectorXd EB(rownania);
			VectorXd EX(rownania);
			for (int row = 0; row < rownania; row++) {
				EB(row) = 0;
				EX(row) = 0;
			}

			int state;
			vector<T> coefficients;    // list of non-zeros coefficients
			SpMat AS(rownania, rownania);
			coefficients.reserve(rownania * 3);
			for (int i = 0; i < rownania; i++)
			{
				//rows
				for (int j = 0; j < l + 1; j++) {
					state = test[i][j].status();
					if (i == test[i][j].wsp) { // jesli wspolczynnik taki sam wierszy to przekatna i 1
						coefficients.push_back(T(i, test[i][j].wsp, 1));
					}
					else if (state == -1) {
						coefficients.push_back(T(i, test[i][j].wsp, -rozklad_rowny[j - 1]));
					}
					else if (state == 1) {
						EB(i) = rozklad_rowny[j - 1];
					}
				}
			}
			AS.setFromTriplets(coefficients.begin(), coefficients.end());
			SparseLU<SparseMatrix<double, ColMajor>, COLAMDOrdering<int> > solver;
			// Compute the ordering permutation vector from the structural pattern of A
			solver.analyzePattern(AS);
			// Compute the numerical factorization
			solver.factorize(AS);
			StartCounter();
			EX = solver.solve(EB);
			czas = GetCounter();
			printf("%-35s: x[0] = %lf  Czas: %lf\n", "SparseLU()", EX(0), czas);

	}
	system("pause");

}
