#include <iostream>
#include <time.h>
#include <locale.h>
#include <stdio.h>
#include <conio.h>

using namespace std;

int maternidade(int lad, float roll);
int leitura();
void prctfinal(float roll, float x, int cont);
int leitura2();
int maternidade2(int dadis, int lad, float roll);
int present()
{
	cout << endl;
	cout << "ROLAGEM DE DADOS ULTRA DELUXE SPECIAL EDITION\n\n";
	cout << "Escolha, digitando o correspondente :\n\n";
	cout << " 1 : Rolar um dado\n ";
	cout << "2 : Rolar mais de um dado\n";
	int option;
	cin >> option;
	if (option == 1)
	{
		return leitura();
	}
	if (option == 2)
	{
		return leitura2();
	}
	else
	{
		present();
	}
}
int leitura()
{
	system("CLS");
	cout << "\nInsira o numero de lados do dado :\n";
	int lad;
	cin >> lad;
	if (lad < 1 or lad>100)
	{
		leitura();
	}
	else
	{
		cout << "\nInsira a quantidade de vezes que o dado sera rolado :\n";
		float roll;
		cin >> roll;
		if (roll > 0)
		{
			system("CLS");
			return maternidade(lad, roll);
		}
		else
		{
			leitura();
		}
	}
	

}
int maternidade(int lad, float roll)
{
	srand(time(NULL));
	float pct[101] = { 0 };
	for (int i = 0; i < roll; i++)
	{
		int dado;
		dado = rand() % lad + 1;
		pct[dado] = pct[dado] + 1;
	}

	cout << "\nNumero de Lados do Dado : " << lad << endl;
	cout << "\nQuantidade de Vezes Rolado : " << roll << endl;
	cout << "\nPercentual de Vezes que cada numero apareceu :" << endl<<endl;
	
	for (int i = 1; i < lad+1; i++)
	{
		prctfinal(roll,pct[i],i);
	}
	cout << "\n\n\nQuer Rolar de novo ?" << endl<<endl;
		cout << "Escolha, digitando o correspondente :\n\n";
		cout << " 1 : Sim\n ";
		cout << " 2 : Nao\n";
		int op2;
		cin >> op2;
		if (op2 == 1)
		{
			system("CLS");
			present();
		}
		else
		{
			return 0;
		}
}
void prctfinal(float roll, float x, int i )
{
	double pcrt;
	pcrt = x/roll;
	pcrt = pcrt * 100;
	cout << "Numero " << i << " :";
	cout << pcrt << "%\n";
}
int leitura2()
{
	system("CLS");
	cout << "\nA SOMA POSSIVEL TOTAL NAO PODE PASSAR DE 100, EXEMPLO : 4 DADOS DE 30\n\n";
	cout << "\nInsira a quantidade de dados que serao rolados :\n";
	int dadis;
	cin >> dadis;
	if (dadis > 0)
	{
		cout << "\nInsira o numero de lados :\n";
		int lad;
		cin >> lad;
		if (lad < 1 or lad>100)
		{
			leitura2();
		}
		else
		{

			if (dadis * lad > 100)
			{
				leitura2();
			}
			else
			{
				cout << "\nQuantas rolagens ? :\n";
				int roll;
				cin >> roll;
				if (roll > 0)
				{
					system("CLS");
					return maternidade2(dadis,lad, roll);
				}
				else
				{
					leitura2();
				}
			}
		}
	}
	else
	{
		leitura2();
	}
}
int maternidade2(int dadis,int lad, float roll)
{
	srand(time(NULL));
	float pct[101] = { 0 };
	for (int a = 0; a < dadis; a++)
	{
		for (int i = 0; i < roll; i++)
		{
			int dado;
			dado = rand() % lad + 1;
			pct[dado] = pct[dado] + 1;
		}
	}
	cout << "\nNumero de Dados : " << dadis << endl;
	cout << "\nNumero de Lados do Dado : " << lad << endl;
	cout << "\nQuantidade de Vezes Rolado : " << roll << endl;
	cout << "\nPercentual de Vezes que cada numero apareceu :" << endl << endl;

	for (int i = 1; i < lad + 1; i++)
	{
		prctfinal(roll*dadis, pct[i], i);
	}
	cout << "\n\n\nQuer Rolar de novo ?" << endl << endl;
	cout << "Escolha, digitando o correspondente :\n\n";
	cout << " 1 : Sim\n ";
	cout << " 2 : Nao\n";
	int op2;
	cin >> op2;
	if (op2 == 1)
	{
		system("CLS");
		present();
	}
	else
	{
		return 0;
	}
}

int main()
{
	present();
}