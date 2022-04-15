#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#define BLANC 255.0

#include <iostream>
#include <filesystem>
#include <vector>
#include <random>
#include <utility>
#include <string>
#include <algorithm>
#include <chrono>
#include <ctime>
#define cimg_display 2
#define cimg_use_png
#include "CImg.h"
#include <stdio.h>
#include <math.h>
#include "ReseauMulticouche.h"
#include <fstream>
#include <numeric>


using namespace cimg_library ;
using namespace std;

template <typename T>
void principale(ifstream* iflux=nullptr, string fichier="");

#include "Affichage.tpp"

#endif // AFFICHAGE_H
