//Nicolae Alina-Elena, 311CA


#define __USE_XOPEN  
#define _GNU_SOURCE // folosite pentru a putea utiliza functia mktime

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


union record //uniunea in care sunt retinute datele despre fisiere si continutul lor
{
	char member[512]; //folosit pentru a retine continutul fisierului
	
	struct header //folosita pentru a retine datele despre fisiere
	{
		char name[100];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char typeflag;
		char linkname[100];
		char magic[8];
		char uname[32];
		char gname[32];
		char devmajor[8];
		char devminor[8];
		
	} header;
	
} block;


struct tm time_info; //structura specifica pentru functia mktime


unsigned int suma () //functie pentru calcularea lui chksum (suma octetilor din header)
{ 
	unsigned int suma = 0; 
	int i; 
	
	for (i = 0 ; i < 512 ; i++) 
	{ 
		suma = suma + (unsigned int) block.member[i]; 
	}
	
	return suma; 
}


int mode (char *t) //functie pentru a calcula in octal permisiunile unui fisier
{
	int nr = 0, nr1 = 0, nr2 = 0; /*se calculeaza permisiunile pentru user, grup 
si altii, separat*/
	
	if (t[1] == 'r') /*se compara, pe rand, fiecare caracter din sirul t cu 
cele trei litere specifice permisiunilor : r (read), w (write), x (execute)*/
	{
		nr += 4;
	}
	
	if (t[2] == 'w')
	{
		nr += 2;
	}
	
	if (t[3] == 'x')
	{
		nr += 1;
	}
	
	if (t[4] == 'r')
	{
		nr1 += 4;
	}
	
	if (t[5] == 'w')
	{
		nr1 += 2;
	}
	
	if (t[6] == 'x')
	{
		nr1 += 1;
	}
	
	if (t[7] == 'r')
	{
		nr2 += 4;
	}
	
	if (t[8] == 'w')
	{
		nr2 += 2;
	}
	
	if (t[9] == 'x')
	{
		nr2 += 1;
	}
	
	return nr*100 + nr1*10 + nr2; //se returneaza numarul in octal
}
	

void load (char t[30]) /*functie care este apelata in momentul introducerii 
comenzii load archivename*/
{
	int i, mod, timp; 
/*i este contor, in mod se calculeaza rezultatul apelarii functiei mode(), in 
timp se calculeaza rezultatul functiei mktime*/
	unsigned int sum = 0; // in sum se retine suma octetilor
	char line[100], line2[100], delim[2] = " ", delim2[2] = ":", *a, *z, d = '-', *c, *e, *g;
//prin intermediul lui line si line2 se parcurg liniile fisierelor
//delim si delim2 reprezinta separatorii folositi in functia strtok

//se aloca dinamic pointerii	
	a = (char *) malloc (20 * sizeof (char));
	c = (char *) malloc (20 * sizeof (char));
	z = (char *) malloc (20 * sizeof (char));
	e = (char *) malloc (20 * sizeof (char));
	g = (char *) malloc (20 * sizeof (char));
		
	FILE *fp = fopen (t, "wb+"); /*se creeaza fisierul "archivename" (reprezentat 
de variabila t) care va deveni arhiva*/
	if ( fp == NULL )
	{
		return;
	}
	
	FILE *fl = fopen ("file_ls", "rt+");
	if ( fl == NULL )
	{
		return;
	}

/*cat timp exista linii in fisierul file_ls care pot fi citite, se verifica daca 
acestea sunt fisiere*/
	while (fgets (line, 100, fl)) //cu line se parcurg liniile fisierului file_ls
	{
		if (*line == d)
		{
			for (i = 0 ; i < 512 ; i++) //se initializeaza blocul header cu 0
			{
				block.member[i] = '\0';
			}
			
			for (i = 0 ; i < 8 ; i++) //se initializeaza chksum cu spatii
			{
				block.header.chksum[i] = ' ';
			}
			
			block.header.typeflag = '0';	
			sprintf (block.header.magic, "%s", "GNUtar ");
		
			a = strtok (line, delim); /*se retine primul camp din liniile din 
file_ls, pe masura ce acestea sunt parcurse*/
			
			int n = 1; /*n marcheaza numarul campului la care s-a ajuns prin 
functia strtok*/
		
			while (n <= 9) /*exista maxim 9 campuri in urma comenzii 
ls -la --time-style=full-iso*/
			{				
				if (n == 1)
				{
					sprintf (z, "%s", a); //se retine primul camp in variabila z
				}
				
				if (n == 3)
				{
					sprintf (block.header.uname, "%s", a); /*al treilea camp, 
reprezentand numele userului, e scris in campul uname din block.header*/
				}
			
				if (n == 4)
				{
					sprintf (block.header.gname, "%s", a); /*al patrulea camp 
corespunde numelui grupului*/
				}
		
				if (n == 5)
				{
					sprintf (block.header.size, "%011o", atoi(a)); /*al 5-lea 
camp corespunde lui size*/
				}
				
				if (n == 6) /*campul 6, corespunzator datei, e retinut in 
variabila e, pentru a putea fi apelat ulterior*/
				{
					strcpy (e, a);
				}
				
				if (n == 7) /*campul 7, corespunzator timpului, e retinut 
in variabila g, pentru a putea fi apelat ulterior*/
				{
					strcpy (g, a);					
				}
			
				if (n == 9) //al 9-lea camp corespunde numelui fisierului
				{
					sprintf (block.header.name, "%s", strtok (a," \n"));
					sprintf (block.header.linkname, "%s", block.header.name);	
				}
				
				a = strtok (NULL, delim);	
				n++;
			}
			
			FILE *fn = fopen ("usermap.txt", "r+t");
			if ( fn == NULL )
			{
				return;
			}
						
			fgets (line2, 100, fn); //cu line2 se parcurg liniile fisierului usermap.txt
			
			c = strtok (line2, delim2);	/*in c se retine primul camp din liniile 
lui usermap.txt, pe masura ce acestea sunt parcurse*/
			
/*in cadrul lui while se parcurg toate liniile, pana cand se gaseste numele 
userului corespunzator fisierului curent*/
			while (strcmp (c, block.header.uname)) 
			{
			    fgets (line2, 100, fn); //se trece la linia urmatoare
			    c = strtok (line2, delim2);

/*cand se gaseste userul potrivit, se parcurg campurile prin strtok pana cand se 
ajunge la campurile 3 (pentru idiul userului), respectiv 4 (pentru idiul grupului)*/
				if (strcmp (c, block.header.uname) == 0) 
				{
					c = strtok (NULL, delim2);
					c = strtok (NULL, delim2);
					sprintf (block.header.uid, "%07o", atoi (c));
					c = strtok (NULL, delim2);
					sprintf (block.header.gid, "%07o", atoi (c));
					break;				
				}
			}
			
			fclose(fn);
			
			mod = mode (z); //se calculeaza permisiunile in octal
			sprintf (block.header.mode, "%07d", mod);

/*se apeleaza functia strptime pentru a face corespondenta intre an, luna si zi, 
respectiv ora si minute cu datele din structura time_info*/
			strptime (e, "%Y-%m-%d", &time_info);
			strptime (g, "%H:%M", &time_info);
			timp = mktime (&time_info);	/*se apeleaza functia mktime pentru a 
calcula numarul de secunde de la 1 Ianuarie 1900 pana la data creerii fisierului 
curent*/
			sprintf (block.header.mtime, "%011o", timp);
			
			sum = suma (); //se calculeaza suma octetilor din header
			sprintf (block.header.chksum, "%06o", sum);

			fwrite (block.member, 512, 1, fp); //se scrie headerul in arhiva
		
			FILE *f = fopen (block.header.name, "r+b"); /*se deschide fisierul 
curent pentru a-i copia continutul in alte blocuri*/
			if (f == NULL)
			{
				return;
			}
		
			while ( !feof(f) ) 
			{
				for (i = 0 ; i < 512 ; i++) /*se copiaza informatia din fisier 
in blocuri (oricat de multe sunt necesare), initial initializate cu 0*/
				{
					block.member[i] = 0;
				}
				fread (block.member, 512, 1, f); 
				fwrite (block.member, 512, 1, fp);
			}
		
			fclose (f);	
		}
	}
	
	for (i = 0 ; i < 512 ; i++) /*la sfarsitul arhivei se introduc doua blocuri 
initializate cu 0*/
	{
		block.member[i] = 0;
	}
	
	fwrite (block.member, 512, 1, fp);
	fwrite (block.member, 512, 1, fp);

	fclose (fl);
	fclose (fp);
}		


void list (char t[30]) /*functia care este apelata in momentul introducerii 
comenzii list archivename*/
{
	long int n;
	
	FILE *fo = fopen (t, "r+b"); //se deschide fisierul arhivei
	
	while ( !feof (fo) ) //se parcurg liniile din arhiva 
	{
		fread (&block, 512, 1, fo); //se parcurge o linie
		printf ("%s\n", block.header.name); //se afiseaza numele fisierului
		n = strtol (block.header.size, NULL, 8); /*se retine in n marimea 
fisierului, pentru a cunoaste cate blocuri trebuie ignorate pana la urmatorul 
header*/
		
		while (n > 512) /*daca marimea fisierului e mai mare de 512 octeti, se 
scriu blocuri*/
		{
			fread (&block, 512, 1, fo);
			n -= 512;
		}
		
		if ( n < 512) /*daca marimea fisierului e mai mica de 512 octeti, se 
scrie un bloc*/
		{
			fread (&block, 512, 1, fo);
		}
	}
	
	fclose (fo);
}

void filename (char t[30], char r[30]) /*functia apelata in momentul introducerii 
comenzii get archivename filename*/
{
	FILE *fo = fopen (t, "r+b"); //se deschide fisierul arhivei
	int n;
	
	while ( !feof(fo) )
	{
		fread (&block, 512, 1, fo); //se citeste fiecare linie
	
		if (strcmp (block.header.name, r) == 0) /*se verifica daca numele 
fisierului corespunde cu filename (reprezentat prin variabila r)*/
		{
			n = strtol (block.header.size, NULL, 8); /*daca da, se retine marimea 
fisierului si se parcurg blocurile cu continutul fisierului pentru ca acesta sa 
poata fi afisat*/
			
			while ( !feof(fo) && n > 0)
			{
				fread (&block, 512, 1, fo);
				printf ("%s", block.member);
				n -= 512;
			}
		}
		
		else //altfel, se mai citeste o linie din fisierul arhivei
		{
			fread (&block, 512, 1, fo);
		}
	}
	
	fclose (fo);
}	
		
	
int main (void)
{
	char s[30], t[30], r[30]; 
//s este primul parametru, t al doilea parametru, r al treilea parametru al comenzii

	scanf ("%s", s);
	
	while ( strcmp (s, "quit") != 0) /*cat timp s este diferit de "quit", se pot 
introduce comenzi de la tastatura*/
	{
		if ( strcmp (s, "load") == 0 ) //pentru s = "load", se apeleaza functia load
		{ 
			scanf ("%s", t);
			load (t);
		}
		else
		{
			if (strcmp (s, "list") == 0) //pentru s = "list", se apeleaza functia list
			{
				scanf ("%s", t);
				list (t);
			}
			else
			{
				if (strcmp (s, "get") == 0) //pentru s = "get", se apeleaza functia get
				{
					scanf ("%s", t);
					scanf ("%s", r);
					filename (t, r);
				}
				else
				{
					if (strcmp (s, "quit") == 0)
					{
						return 0;
					}
				}
			}
		}
		
		scanf ("%s", s);
	}
		
	return 0;
}
		
			
