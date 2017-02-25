#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdlib>

//Change these values according to your image
#define IMAGE_HEIGHT 32 
#define IMAGE_WIDTH  32

void decode(std::string, int*, int*);

int main()
{
	std::string line;
	std::ifstream file ("pic.txt"); //Input file
	
	if(!file.is_open())
	{
		std::cout << "Error opening file." << std::endl;
		exit(1);
	}

	std::string delimiter = ", ";
	size_t pos = 0;
	std::string token;
	int hex = 0x00, add = 128, counter = 0, inc = 0;
	int image_area = IMAGE_HEIGHT * IMAGE_WIDTH;
	int arr_size = image_area / 8;
	
	if(image_area % 8 > 0)
		arr_size += 1;
		
	int hexarr[arr_size]; 

	for(int i=0; i < IMAGE_HEIGHT; i++)
	{
		std::getline(file, line);

		while((pos = line.find(delimiter)) != std::string::npos)
		{
			token = line.substr(0, pos);		
			decode(token, &hex, &add);
			
			line.erase(0, pos + delimiter.length());
			if(counter < 7)
			{			
				counter++;
				add /= 2;
			}
			else
			{
				counter = 0;
				add = 128;
				hexarr[inc++] = hex;
				hex = 0;
			}	
		}

		if(i == IMAGE_HEIGHT-1)
		{
			decode(line, &hex, &add);
			hexarr[inc] = hex;

			if(image_area % 8 > 0)
				hexarr[inc] << image_area % 8;
		}
			
		std::cout << "\n";
	}
	
	file.close();

	std::ofstream outfile ("hex.txt"); //Output file

	if(!outfile.is_open())
	{
		std::cout << "Error opening file." << std::endl;
		exit(1);
	}

	for(int i=0; i < arr_size; i++)
	{
		std::cout << std::showbase << std::internal << std::setfill('0');
		outfile << std::showbase << std::internal << std::setfill('0');
		if(hexarr[i] != 0)
		{
			std::cout << std::hex << std::setw(4) << hexarr[i] << " ";
			outfile << std::hex << std::setw(4) << hexarr[i];
		}
		else
		{
			std::cout << "0x00 ";
			outfile << "0x00";
		}
		outfile << ", ";
		if((i+1) % 12 == 0)
		{
			std::cout << "\n";
			outfile << "\n";
		}
		else if(i == arr_size - 1)
			std::cout << "\n";
	}
	
	outfile.close();

	return 0;	
}

void decode(std::string parsed_string, int *hex, int *add)
{
	if(parsed_string == "0x00000000")
	{
		std::cout << "0";
	}
	else if (parsed_string == "0xff000000")
	{
		std::cout << "1";
		*hex += *add;
	}
}
