/***************************************************************************
 *   Copyright 2007      Francesco Rossi <redsh@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "stdio.h"

int w = 21; //width
int h = 21; //height
int o = 9;  //order

int index(int r, int c)
{
	return w*r+c;
}

int row(int index)
{
	return index/w;
}

int col(int index)
{
	return index%w;
}

void print_head()
{
	printf("<clique size=\"%d\" >", o);
}

void print_tail()
{
	printf("</clique>\n");
}

void print_row(int r, int c) //rc row column
{
	print_head();
	int s0 = r*w+c;
	for(int j=0; j<o; j++)
	{
		printf("%d ", s0+j);
	}
	print_tail();
}

void print_col(int r, int c)
{
	print_head();
	for(int j=0; j<o; j++)
	{
		printf("%d ", index(r+j,c));
	}
	print_tail();
}

void print_3x3(int r0, int c0)
{
	print_head();
	int r,c;
	for(r=0; r<3; r++)		
	{
		for(c=0; c<3; c++)
		{
			printf("%d ", index(r0+r, c0+c));
		}
	}
	print_tail();
}

void print_9x9_sudoku(int r0, int c0)
{
	int i,j;
	for(i=0; i<9; i++)
	{
		print_row(r0+i,c0);
	}
	for(i=0; i<9; i++)
	{	
		print_col(r0,c0+i);
	}
	for(i=0; i<9; i++)
	{	
		print_3x3(r0+(i/3)*3,c0+(i%3)*3);
	}
}

int main(void)
{
	print_9x9_sudoku(0,0);
	printf("\n");
	print_9x9_sudoku(0,12);
	printf("\n");
	print_9x9_sudoku(12,0);
	printf("\n");
	print_9x9_sudoku(12,12);
	printf("\n");
	print_9x9_sudoku(6,6);
	printf("\n");

}
