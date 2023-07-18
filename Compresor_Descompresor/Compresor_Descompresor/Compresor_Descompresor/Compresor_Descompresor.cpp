#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <stdio.h>

#pragma warning(disable:4996);

#define ll unsigned long long int //se utiliza para que cada int en toda la función se convierta en long long y no tengamos que escribirlo una y otra vez.
// ll es el sufijo de long-long, que es de 64 bits en la mayoría, LL es un literal de 64 bits con el valor 0.
// La palabra clave unsigned es un especificador de tipo de datos, que hace que una variable sólo represente números enteros no negativos (números positivos y cero).

using namespace std;
using namespace std::chrono;

/*
	Libreria de compresion
*/
namespace Huffman { // Un espacio de nombres es una región declarativa que proporciona un ámbito a los identificadores(los nombres de tipos, funciones, variables, etc.)

	string HuffmanValue[256] = { "" };

	typedef struct Node {
	public:
		char character; //caracter
		ll count; //cantidad de bits que aparece
		Node* left, * right;//para crear el nodo

		Node(ll count) {
			this->character = 0;
			this->count = count;
			this->left = this->right = nullptr;
		}

		Node(char character, ll count) {
			this->character = character;
			this->count = count;
			this->left = this->right = nullptr;
		}
	} Node;
	/*
	*   Espacio en comun para las funciones necesarias de compresor y descompresor
	*/
	namespace Utility { // Un espacio de nombres es una región declarativa que proporciona un ámbito a los identificadores(los nombres de tipos, funciones, variables, etc.)

		ll GetFileSize(const char* filename) { // le llega el nombre
			FILE* p_file = fopen(filename, "rb"); //abre el archivo
			fseek(p_file, 0, SEEK_END); //La función fseek() establece el indicador de posición del archivo para el flujo de archivos dado.
			ll size = _ftelli64(p_file); //devuelve la posición actual del archivo
			fclose(p_file); //cierra el archivo
			return size; //devuelve el tamaño del archivo
		}

		//// Función de prueba para imprimir los códigos huffman para cada carácter.
		//void Inorder(Node* root, string& value) {
		//	if (root) {
		//		value.push_back('0');
		//		Inorder(root->left, value);
		//		value.pop_back();
		//		if (!root->left && !root->right) {
		//			printf("Character: %c, Count: %lld, ", root->character, root->count);
		//			cout << "Huffman Value: " << value << endl;
		//		}
		//		value.push_back('1');
		//		Inorder(root->right, value);
		//		value.pop_back();
		//	}
		//}
	};

	/*
	*	Funciones necesarias para el compresor
	*/
	namespace CompressUtility { // Un espacio de nombres es una región declarativa que proporciona un ámbito a los identificadores(los nombres de tipos, funciones, variables, etc.)
		/*
			Combina dos nodos
		*/
		Node* Combine(Node* a, Node* b) 
		{
			Node* parent = new Node((a ? a->count : 0) + (b ? b->count : 0));
			parent->left = b;
			parent->right = a;
			return parent;
		}

		bool sortbysec(Node* a, Node* b) {
			return (a->count > b->count);
		}
		// Pase inicial para contar caracteres.
		map <char, ll> ParseFile(const char* filename, ll Filesize) // llegan el archivo y el tamaño del archivo
		{//map (Los mapas son contenedores asociativos que almacenan elementos formados por la combinación de un valor clave y un valor mapeado, siguiendo un orden específico.)
		
			register FILE* ptr = fopen(filename, "rb");//abre el archivo en lectura

			if (ptr == NULL) 
			{ //si no encuentra el archivo
				perror("Error: File not found:");
				exit(-1);
			}
			register unsigned char ch; // para obtener los elementos del archivo // es un tipo de datos de caracteres en el que la variable consume los 8 bits 
			// de la memoria y no hay bit de signo (que sí hay en los char con signo). Esto significa que el rango del tipo de datos unsigned char va de 0 a 255.
			
			register ll size = 0, filesize = Filesize; // La palabra clave register es un especificador de tipo de almacenamiento //filesize es el tamaño del archivo
			
			vector<ll>Store(256, 0); //vector, Los vectores son un tipo de array, va a guardar datos tipo ll

			while (size != filesize) // mientras que no se llegue al tamaño del archivo se hace el ciclo
			{ 
				ch = fgetc(ptr); // Devuelve el carácter apuntado actualmente por el indicador de posición de archivo interno del flujo especificado
				++Store[ch]; // le suma al array del que almacena los bits [a los elementos de archivo que serian los bits]
				++size; // suma para el ciclo
			}

			map <char, ll> store;// contiene char y los bits

			for (int i = 0; i < 256; ++i) // ciclo
				if (Store[i]) // si hay para almacenar
					store[i] = Store[i]; // se almacena
			fclose(ptr); // cierra el archivo
			return store; //los caracteres contados en bits
		}


		vector <Node*> SortByCharacterCount(const map <char, ll >& value) 
		{
			vector < Node* > store; //array de nodos
			//auto =  la palabra clave auto declara una variable en la clase de almacenamiento automática; es decir, una variable que tiene una duración local.
			auto it = begin(value); //Devuelve un iterador que apunta al primer elemento de la secuencia
			for (; it != end(value); ++it) // ciclo el inicio hasta el final 
				store.push_back(new Node(it->first, it->second)); // una función predefinida que se utiliza para insertar datos o elementos al final del vector o empuja el elemento en el vector desde atrás.
			sort(begin(store), end(store), sortbysec);// ordena del menor al mayor

			//begin () Devuelve un iterador que apunta al primer elemento de la secuencia
			//end () Devuelve un iterador que apunta al elemento pasado del final de la secuencia

			return store; // el array ordenado
		}


		// Generar una cabecera para el archivo.
		// Formato: 
		// 1. Total de caracteres únicos (1 byte)
		// 2. Para cada carácter único:
		// 2a. Carácter (1 byte)
		// 2b. Longitud del código (1 byte)
		// 2c. Código Huffman (mínimo: 1 byte, máximo: 255bytes)
		// 3. Relleno
		// Tamaño de la cabecera en el peor de los casos: 1 + (1+1)*(1+2+3+4+5+...+255) + 1 ~ 32kb... (sólo ocurre cuando se genera un árbol Huffman sesgado)
		// Tamaño de la cabecera en el mejor de los casos: 1 + 1 + 1 + 1 = 5bytes (Sólo ocurre cuando existe un único carácter en todo el archivo).

		string GenerateHeader(char padding) 
		{
			string header = "";

			//UniqueCharacter empieza por - 1 {0 significa 1, 1 significa 2, para conservar la memoria}
			unsigned char UniqueCharacter = 255;

			for (int i = 0; i < 256; ++i) {
				if (HuffmanValue[i].size()) {
					header.push_back(i); // se inserta i 
					header.push_back(HuffmanValue[i].size()); // se inserta el tamaño del elemento que este en i
					header += HuffmanValue[i]; // se suma el elemento que este en i
					++UniqueCharacter; // se suma
				}
			}
			char value = UniqueCharacter; // para sumar todo al final

			return value + header + (char)padding; // se suma todo
		}

		// Almacena los valores Huffman para cada carácter de la cadena.
		// devuelve el tamaño del archivo resultante (sin la cabecera)
		ll StoreHuffmanValue(Node* root, string& value) {
			ll temp = 0; // variable tipo bits
			if (root) // mientras que el nodo sea true
			{
				value.push_back('0'); // inserta 0
				temp = StoreHuffmanValue(root->left, value); // llamado recursivo
				value.pop_back(); // elimina elemento

				if (!root->left && !root->right) // si son diferentes de true (son false)
				{
					HuffmanValue[(unsigned char)root->character] = value; // al string principal se le asigna el caracter que contiene el valor
					temp += value.size() * root->count; // se suma el tamaño de value x los bits del nodo
				}

				value.push_back('1'); // se inserta 1
				temp += StoreHuffmanValue(root->right, value); // se suma el llamado recursivo del nodo derecha y el valor
				value.pop_back(); // se elimina
			}
			return temp; // se duelve la variable
		}
		// Crear el árbol huffman durante la compresión...
		Node* GenerateHuffmanTree(const map <char, ll> & value) 
		{
			vector < Node* > store = SortByCharacterCount(value); // crea un array de nodos ordenados, se envia los caracteres contados en bits
			Node* one, * two, * parent;//se crean nodos

			sort(begin(store), end(store), sortbysec); // lo ordena
			if (store.size() == 1) // si el tamaño del array es igual a 1
			{
				return Combine(store.back(), nullptr); // back() devuelve una referencia al último elemento del vector, combine combina dos nodos
			}
			while (store.size() > 2) // si el tamaño del array es mayor a 2
			{
				//nodo one y two
				one = *(end(store) - 1); two = *(end(store) - 2); // end() Devuelve un iterador que apunta al elemento pasado en la secuencia
				parent = Combine(one, two); // combina los nodos
				store.pop_back(); store.pop_back(); // pop_back()  se utiliza para eliminar un elemento de la parte posterior de un contenedor de lista.
				store.push_back(parent); // push_back() es una función predefinida que se utiliza para insertar datos o elementos al final del vector o empuja el elemento en el vector desde atrás.

				// iterator (Un iterador es un objeto que puede iterar sobre los elementos de un contenedor de la biblioteca estándar de C++ y proporcionar acceso a los elementos individuales)
				vector <Node*> ::iterator it1 = end(store) - 2;
				while ((*it1)->count < parent->count && it1 != begin(store)) //mientras que el array sea menor a los bits contados del nodo y el array sea diferente de el primer iterador 
					//de la secuencia
					--it1;

				sort(it1, end(store), sortbysec); // se ordena
			}
			one = *(end(store) - 1); two = *(end(store) - 2); // one (ultimo iterador menos 1) //two (ultimo iterador menos 2)
			parent = Combine(one, two); // se combinan
			return parent; //devuelve el nodo
		}

		/*
			Compresion actual del archivo
		*/
		void Compress(const char* filename, const ll Filesize, const ll PredictedFileSize) 
		{
			const char padding = (8 - ((PredictedFileSize) & (7))) & (7);// se asigna un char de los bits nuevos

			string header = GenerateHeader(padding); // se genera una cabecera para el archivo

			int header_i = 0;
			const int h_length = header.size(); // tamaño de la cabecera

			cout << "Tamano de la cabecera: " << (int)padding << endl; // se imprime

			FILE* iptr = fopen(filename, "rb"), * optr = fopen((string(filename) + ".abiz").c_str(), "wb"); // se abre el archivo y el archivo donde se va a comprimir

			while (header_i < h_length) { // mientras que 0 sea menor a al tamaño de la cabecera
				fputc(header[header_i], optr); // Escribe un carácter en el flujo y hace avanzar el indicador de posición.

				++header_i;//se suma
			}

			if (!iptr) { // no encuentra el archivo
				perror("Error: File not found: ");
				exit(-1);
			}

			unsigned char ch, fch = 0; // specificador de tipo de datos, que hace que una variable sólo represente números enteros no negativos
			char counter = 7;
			ll size = 0, i; // variable tipo bits

			while (size != Filesize) // mientras que size sea menor al tamaño del archivo original
			{
				ch = fgetc(iptr);// Devuelve el carácter apuntado actualmente por el indicador de posición de archivo interno del flujo especificado
				i = 0;
				while (HuffmanValue[ch][i] != '\0') // mientras que el string prinicpal en las posiciones [ch][i] sea diferente de \0
				{
					fch = fch | ((HuffmanValue[ch][i] - '0') << counter); // los bits se igualan a string prinicpal en las posiciones [ch][i] menos "0" y se mueven 7 espacios (o los espacios que tenga counter)
					--counter; // se resta
					if (counter == -1) { // si counter llega a -1
						fputc(fch, optr); // Escribe un carácter en el flujo y hace avanzar el indicador de posición. en el archivo nuevo
						counter = 7; 
						fch = 0;
					}
					++i;
				}
				++size;

				if (((size * 100 / Filesize)) > ((size - 1) * 100 / Filesize))
					printf("\r%d%% Completado  ", (size * 100 / Filesize)); // imprime que se completo
			}
			if (fch)
				fputc(fch, optr); // Escribe un carácter en el flujo y hace avanzar el indicador de posición. en el archivo nuevo
			printf("\n");
			fclose(iptr); // cierra
			fclose(optr); // cierra
		}

	};
	/*
	*	Funciones necesarias para el descompresor
	*/
	namespace DecompressUtility 
	{

		// Crea un arbol de huffman durante la descompresion
		void GenerateHuffmanTree(Node* const root, const string& codes, const unsigned char ch) 
		{
			Node* traverse = root; // nodo
			int i = 0;

			while (codes[i] != '\0') // mientras que el elemento que este en codes[i] sea diferente de '\0'
			{
				if (codes[i] == '0') { // si el elemento que este en codes[i] es igual a '\0'
					if (!traverse->left) // traverse->left == false
						traverse->left = new Node(0); // se hace nuevo nodo
					traverse = traverse->left; 
				}
				else {
					if (!traverse->right) // traverse->right == false
						traverse->right = new Node(0); // se hace nuevo nodo
					traverse = traverse->right;
				}
				++i;
			}
			traverse->character = ch; // el caracter que almacena el nodo 
		}

		// Función para almacenar y generar un árbol
		pair<Node*, pair<unsigned char, int> >DecodeHeader(FILE* iptr) 
		{

			Node* root = new Node(0); // nodo
			int charactercount, buffer, total_length = 1;
			register char ch, len; // register La palabra clave register es un especificador de tipo de almacenamiento
			charactercount = fgetc(iptr); // Devuelve el carácter apuntado actualmente por el indicador de posición de archivo interno del flujo especificado
			string codes;
			++charactercount;

			while (charactercount) 
			{
				ch = fgetc(iptr); // Devuelve el carácter apuntado actualmente por el indicador de posición de archivo interno del flujo especificado
				codes = "";
				buffer = 0;
				len = fgetc(iptr);  // Devuelve el carácter apuntado actualmente por el indicador de posición de archivo interno del flujo especificado
				buffer = len;

				while (buffer > codes.size())
					codes += fgetc(iptr);  // Devuelve el carácter apuntado actualmente por el indicador de posición de archivo interno del flujo especificado y se suma

				// carácter (1byte) + longitud(1byte) + huffmancode(n bytes donde n es la longitud del huffmancode)
				total_length += codes.size() + 2;

				GenerateHuffmanTree(root, codes, ch); // se manda a generar un arbol con el nodo, los codigos y char
				--charactercount;
			}
			unsigned char padding = fgetc(iptr);
			++total_length;

			return { root, {padding, total_length} }; // se duelve el nodo, con la cabecera y el total del tamaño
		}
		// Actual decompression function
		void Decompress(const char* filename, const ll Filesize) 
		{
			string fl = string(filename);//nombre del archivo
			FILE* iptr = fopen(fl.c_str(), "rb"); // se abre el archivo // c_str() que devuelve un puntero a una matriz que contiene una secuencia de caracteres terminada en cero que representa el valor actual del objeto basic_string
			FILE* optr = fopen(string("output" + fl.substr(0, fl.length() - 5)).c_str(), "wb"); // archivo el cual va a salir descomprimido

			if (iptr == NULL) {
				perror("Error: File not found");
				exit(-1);
			}

			// pair (Pair se utiliza para combinar dos valores que pueden ser de diferentes tipos de datos. Pair proporciona una forma de almacenar dos objetos heterogéneos como una sola unidad)
			pair<Node*, pair<unsigned char, int> >HeaderMetadata = DecodeHeader(iptr); // almacena y genera un arbol

			//Los nodos
			Node* const root = HeaderMetadata.first; // primer nodo
			const auto padding = HeaderMetadata.second.first; // segundo nodo, la cabecera
			const auto headersize = HeaderMetadata.second.second; // segundo segundo nodo, tamaño de la cabecera

			char ch, counter = 7;
			ll size = 0; //bits
			const ll filesize = Filesize - headersize; // tamaño del archivo menos la cabecera
			Node* traverse = root; // nodo

			ch = fgetc(iptr); // Devuelve el carácter apuntado actualmente por el indicador de posición de archivo interno del flujo especificado

			while (size != filesize)
			{
				while (counter >= 0) 
				{
					traverse = ch & (1 << counter) ? traverse->right : traverse->left; // traverse = char y las posiciones del counter y lo que tenga traverse->right y traverse->left
					ch ^= (1 << counter); // ch = elevado a lo que de las posiciones el counter
					--counter;

					if (!traverse->left && !traverse->right) // si son diferentes de falso
					{
						fputc(traverse->character, optr); // Escribe un carácter en el flujo y hace avanzar el indicador de posición.
						if (size == filesize - 1 && padding == counter + 1) // punto de salida de este ciclo
							break;
						traverse = root; // vuelve a ser el nodo primer nodo
					}
				}
				++size;
				counter = 7;

				if (((size * 100 / filesize)) > ((size - 1) * 100 / filesize))
					printf("\r%lld%% Completado, Tamano: %lld bytes   ", (size * 100 / filesize), size); // imprime que se completo

				ch = fgetc(iptr); // Devuelve el carácter apuntado actualmente por el indicador de posición de archivo interno del flujo especificado
			}
			fclose(iptr); // se cierra
			fclose(optr); // se cierra
		}
	};
};

using namespace Huffman;

int main(int argc, char* argv[]) {

	if (argc != 3) {//verifica que se den bien los argumentos
		printf("Usage:\n (a.exe|./a.out) (-c FileToBeCompressed | -dc FileToBeDecompressed)");
		exit(-1);
	}
	const char* option = argv[1], * filename = argv[2];//guarda en una variable los argumentos dados, si es comprimir/descomprimir y el tipo de archivo
	printf("Nombre del archivo a Comprimir/Descomprimir: %s\n", filename);//imprime el nombre

	time_point <system_clock> start, end;//para contar el tiempo que dura en realizarse
	duration <double> time;

	ll filesize, predfilesize;//para obtener lo que pese el archivo y lo que va a terminar pesando luego

	if (string(option) == "-c") { //Opcion comprimir

		filesize = Utility::GetFileSize(filename);//obtiene el tamaño del archivo
		auto mapper = CompressUtility::ParseFile(filename, filesize); //auto = forma sencilla de declarar una variable que tiene un tipo complicado 

		// mapper serian los caracteres contados en bits
		Node* const root = CompressUtility::GenerateHuffmanTree(mapper); // genera el arbol de huffman con los bits de los caracteres
		string buf = "";

		// Funcion para que Almacena los valores Huffman para cada carácter de la cadena.
		// devuelve  tamaño del archivo resultante (sin la cabecera)
		predfilesize = CompressUtility::StoreHuffmanValue(root, buf); // se envia el nodo y un string para escribir
		//predfilesize (serian los bits nuevos)

		printf("Archivo Original: %lld bytes\n", filesize); // se imprime lo que pesaba antes el archivo sin descomprimir
		printf("Tamano del archivo comprimido (Sin la cabecera): %lld bytes\n", (predfilesize + 7) >> 3); // se imprime el peso nuvo del archivo comprimido 

		start = system_clock::now();//empieza tiempo

		//Funcion para comprimir el archivo, se manda el nombre, lo que pesaba y el nuevo peso
		CompressUtility::Compress(filename, filesize, predfilesize);

		end = system_clock::now();//termina tiempo

		time = (end - start);
		cout << "Tiempo de compresion: " << time.count() << "s" << endl;//tiempo durado del compresor

	}
	else if (string(option) == "-dc") { //Opcion descomprimir

		filesize = Utility::GetFileSize(filename); // obtiene el tamaño del archivo
		start = system_clock::now();
		//Funcion para descomprimir el archivo, se manda el nombre, lo que pesa
		DecompressUtility::Decompress(filename, filesize); 
		end = system_clock::now();

		time = (end - start);
		cout << "\nTiempo de descompresion: " << time.count() << "s" << endl;
	}
	else
		cout << "\nInvalid Option... Exiting\n";
	return 0;

}