#include <filesystem>
#include <cstring>  
#include <fstream>
#include <thread>
#include <cmath>
#include <omp.h>
#include <iostream>

namespace fs = std::filesystem;

std::string* listOfHashesA;
std::string* listOfHashesB;
std::string* listOfHashesC;

int OS = 0;

struct item
{
  int val;
  int posRow;
  int posCol;
};


void printFormat()
{
	std::cout << std::endl << "*** SiSe (Similarity Search) version 1.0 by CSIC-ITEFI ***" << std::endl; 	
	std::cout << "\r\n*******************" << std::endl;
	std::cout << "*** CREATE HASH ***" << std::endl;
	std::cout << "*******************" << std::endl;
	std::cout << "\r\nOption 1:" << std::endl;	
	std::cout << "\r\n> sise input_file(s)" << std::endl;	
	std::cout << "\r\nOption 2:" << std::endl;		
	std::cout << "\r\n> sise -i input_file [-o output_file] [-v num_bytes_window] [-b block_size] [-d decremental_value] [-t num_threads]" << std::endl;
	std::cout << "\r\nDefault values:" << std::endl;
	std::cout << "  o: in addition to showing the result on the screen, the program stores the hash in the output file" << std::endl;
	std::cout << "  v: 7 bytes" << std::endl;
	std::cout << "  b: computed by the program" << std::endl;
	std::cout << "  d: 1" << std::endl;
	std::cout << "  t: number of cores" << std::endl;
	std::cout << "\r\n*******************************************************" << std::endl;
	std::cout << "*** CREATE HASH OF ALL THE FILES STORED IN A FOLDER ***" << std::endl;
	std::cout << "*******************************************************" << std::endl;
	std::cout << "\r\n> sise -r input_folder" << std::endl;		
	std::cout << "\r\n****************************" << std::endl;
	std::cout << "*** COMPARE FILES/HASHES ***" << std::endl;
	std::cout << "****************************" << std::endl;
	std::cout << "\r\n> sise -c input_file_1 input_file_2" << std::endl << std::endl;
	std::cout << "(Input files can be either content files or digest files)" << std::endl;
	std::cout << "\r\n*******************************************" << std::endl;
	std::cout << "*** COMPARE ALL HASHES STORED IN A FILE ***" << std::endl;
	std::cout << "*******************************************" << std::endl;
	std::cout << "\r\n> sise -x input_file" << std::endl << std::endl;	
}


item make_item(int val1, int x1, int y1) 
{
    item myitem = {val1, x1, y1};
    return myitem;
}


std::string removeSequences(std::string sInit)
{
	std::string sRes = sInit.substr(0,3);

	int lenx = sInit.size();

	for(int i=3, j=3; i < lenx; i++)
	{
		if(((sInit.substr(i,1)).compare(sInit.substr(i-1))) ||
           ((sInit.substr(i,1)).compare(sInit.substr(i-2))) ||
           ((sInit.substr(i,1)).compare(sInit.substr(i-3))))
		{
			sRes = sRes.append(sInit.substr(i,1));
		}
	}

	return sRes;
}


std::string createHash(const std::string &inputFile, uint64_t tama, int sizeWindow, int block, int dec, bool debug, uint64_t tambuf)
{
	int lB = block;
	int64_t lAi = 0;
	int64_t lBd1 = 0, lBd2 = 0, lBd4 = 0;
	int64_t lMd1 = 0, lMd2 = 0, lMd4 = 0;
	int64_t lDec = dec, lDecd1 = 0, lDecd2 = 0, lDecd4 = 0;
	int64_t h1 = 0, h2 = 0, h3 = 0, resultado = 0;
	int64_t hashPrime = 0x01000193;
	int64_t hashInit = 0x28021967;
	int64_t h22a = hashInit;
	int64_t h22b = hashInit;
	int64_t h22c = hashInit;
	int64_t lBfb = 0;
	int64_t lHia = 0;
	int64_t lHib = 0;
	int64_t lHic = 0;
	int64_t lDecfb = 0;
	int64_t numChars = 64, n = 0, temp = 0;

	char bValor = 0;

	bool Bd1m3 = false, Bd2m3 = false, Bd4m3 = false, noCadena = false;

	std::string base64code = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string sCadenaB64, sCadenaB64a, sCadenaB64b, sCadenaB64c, sCadenaB64fb;
	
	auto * baVentana = new char[sizeWindow];

	for(int i=0;i<sizeWindow;i++)
	{
		baVentana[i]=0;
	}

	auto* memblock = new char[tambuf];
	size_t inFileSize = tama;

    std::ifstream in(inputFile, std::ifstream::binary);

    if (!in.is_open())
    {
        std::cout << "Can't open input file: " << inputFile << std::endl;
        return "Aborted: cannot open file";
    }
    
		
	if (lB <= 0)
	{
		int cociente = std::ceil(((double)inFileSize) / 64);

		lB = 3;
		while (lB < cociente)
		{
			lB = lB * 2;
		}
	}

	lBd1 = lB;
	lBd2 = lB / 2;
	lBd4 = lB / 4;

	lDecd1 = lDec;
	lDecd2 = lDec;
	lDecd4 = lDec;

	if (lBd1 < 3)
	{
		Bd1m3 = true;
	}
	else
	{
		lMd1 = (lBd1 - lDecd1) % lBd1;
	}

	if (lBd2 < 3)
	{
		Bd2m3 = true;
	}
	else
	{
		lMd2 = (lBd2 - lDecd2) % lBd2;
	}

	if (lBd4 < 3)
	{
		Bd4m3 = true;
	}
	else
	{
		lMd4 = (lBd4 - lDecd4) % lBd4;
	}

	while ((sCadenaB64a.length() < numChars) && (sCadenaB64b.length() < numChars) && !(Bd1m3&&Bd2m3&&Bd4m3))
	{
	    in.seekg(0, std::ifstream::beg);

	    for (size_t bytesLeft = inFileSize, chunk = tambuf; bytesLeft > 0; bytesLeft -= chunk)
	    {
	        if (bytesLeft < chunk)
	        {
	            chunk = bytesLeft;
	        }
	        
	        in.read(&memblock[0], chunk);

			for (uint64_t index = 0; index < chunk; index++)
			{
				bValor = memblock[index];
				h2 = h2 - h1;
				h2 = h2 + sizeWindow * (bValor & 0x00FF);
				h2 = h2 & 0x00FFFFFFFF;
				h1 = h1 + (bValor & 0x00FF);
				h1 = h1 - (baVentana[n] & 0x00FF);
				h1 = h1 & 0x00FFFFFFFF;

				baVentana[n] = bValor;
				n = (++n) % sizeWindow;

				h3 = (h3 << 5) & 0x00FFFFFFFF;
				h3 = h3^(bValor&0x00FF);
				h3 = h3 & 0x00FFFFFFFF;

				resultado = (h1 + h2 + h3) & 0x00FFFFFFFF;
				lAi = resultado;

				h22a = 0xFFFFFFFF & (h22a*hashPrime);
				h22a = 0xFFFFFFFF & (h22a ^ (0xFF & bValor));
				
				h22b = 0xFFFFFFFF & (h22b*hashPrime);
				h22b = 0xFFFFFFFF & (h22b ^ (0xFF & bValor));

				h22c = 0xFFFFFFFF & (h22c*hashPrime);
				h22c = 0xFFFFFFFF & (h22c ^ (0xFF & bValor));

				if (!Bd4m3)
				{
					// If /4 is trigger point
					if (((lAi%lBd4) == lMd4))
					{
						lHic = h22c;
						h22c = hashInit;
													
						// Second character
						temp = (int)((lHic >> 6) % 64);
						sCadenaB64c = sCadenaB64c.append(base64code.substr(temp, 1));
													
						// First character    
						temp = (int)(lHic % 64);
						sCadenaB64c = sCadenaB64c.append(base64code.substr(temp, 1));

						// If /2 is trigger point
						if ((!Bd2m3) && (lAi%lBd2) == lMd2)
						{
							lHib = h22b;
							h22b = hashInit;
															
							// Second character
							temp = (int)((lHib >> 6) % 64);
							sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));

							// First character
							temp = (int)(lHib % 64);
							sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));

							// If /1 is trigger point
							if ((!Bd1m3) && ((lAi%lBd1) == lMd1))
							{
								lHia = h22a;
								h22a = hashInit;
																	
								// Second character
								temp = (int)((lHia >> 6) % 64);
								sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
																	
								// First character
								temp = (int)(lHia % 64);
								sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
							} 
						} 
					} 
				} 
				else
				{
					if (!Bd2m3)
					{
						// If /2 is trigger point
						if ((lAi%lBd2) == lMd2)
						{
							lHib = h22b;
							h22b = hashInit;

							// Second character
							temp = (int)((lHib >> 6) % 64);
							sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));
															
							// First character
							temp = (int)(lHib % 64);
							sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));

							// If /1 is trigger point
							if ((!Bd1m3) && ((lAi%lBd1) == lMd1))
							{
								lHia = h22a;
								h22a = hashInit;
																	
								// Second character
								temp = (int)((lHia >> 6) % 64);
								sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
																	
								// First character
								temp = (int)(lHia % 64);
								sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
							} 
						}                         
					} 
					else
					{
						if (!Bd1m3)
						{
							// If /1 is trigger point
							if ((lAi%lBd1) == lMd1)
							{
								lHia = h22a;
								h22a = hashInit;
																	
								// Second character
								temp = (int)((lHia >> 6) % 64);
								sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));

								// First character
								temp = (int)(lHia % 64);
								sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
							}                        
						} 
					} 
				} 
			} 
		} 

		// Last pair
		lHia = h22a;
		
		// Second character
		temp = (int)((lHia >> 6) % 64);
		sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
		
		// First character
		temp = (int)(lHia % 64);
		sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));

		lHib = h22b;
		h22b = hashInit;
		
		// Second character
		temp = (int)((lHib >> 6) % 64);
		sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));
		
		// First character
		temp = (int)(lHib % 64);
		sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));

		lHic = h22c;
		h22c = hashInit;
		
		// Second character
		temp = (int)((lHic >> 6) % 64);
		sCadenaB64c = sCadenaB64c.append(base64code.substr(temp, 1));
		
		// First character
		temp = (int)(lHic % 64);
		sCadenaB64c = sCadenaB64c.append(base64code.substr(temp, 1));

		if (!Bd1m3)
		{
			sCadenaB64fb = sCadenaB64a;
			lBfb = lBd1;
			lDecfb = lDecd1;
		}

		if (!Bd2m3)
		{
			sCadenaB64fb = sCadenaB64b;
			lBfb = lBd2;
			lDecfb = lDecd2;
		}

		if (!Bd4m3)
		{
			sCadenaB64fb = sCadenaB64c;
			lBfb = lBd4;
			lDecfb = lDecd4;
		}

		sCadenaB64a = removeSequences(sCadenaB64a);
		sCadenaB64b = removeSequences(sCadenaB64b);

		if ((sCadenaB64a.length() < numChars) && (sCadenaB64b.length() < numChars))
		{
			noCadena = true;
								
			sCadenaB64a = "";
			sCadenaB64b = "";
			sCadenaB64c = "";

			lBd1 = lBd4;
			lBd2 = lBd1 / 2;
			lBd4 = lBd2 / 2;

			if (lBd1 < 3)
			{
				Bd1m3 = true;
			}
			else
			{
				lMd1 = (lBd1 - lDecd1) % lBd1;
			}

			if (lBd2 < 3)
			{
				Bd2m3 = true;
			}
			else
			{
				lMd2 = (lBd2 - lDecd2) % lBd2;
			}

			if (lBd4 < 3)
			{
				Bd4m3 = true;
			}
			else
			{
				lMd4 = (lBd4 - lDecd4) % lBd4;
			}

		}
		else
		{
			noCadena = false;
		}

	} 

	in.close();

	if (noCadena)
	{
		std::stringstream stringstream1;
		stringstream1 << lBfb << "(" << (lBfb - lDecfb) << "):" << sCadenaB64fb << "::" << lBfb << "(" << (lBfb - lDecfb) << "):" << sCadenaB64fb;
		std::string stringx1 = stringstream1.str();

		return stringx1;
	}

	if (sCadenaB64a.length() >= numChars)
	{
		std::stringstream stringstream2;
		stringstream2 << lBd1 << "(" << ((lBd1 - lDecd1) % lBd1) << "):" << sCadenaB64a << "::" << lBd2 << "(" << ((lBd2 - lDecd2) % lBd2) << "):" << sCadenaB64b;
		std::string stringx2 = stringstream2.str();

		return stringx2;
	}
	else
	{
		std::stringstream stringstream3;
		stringstream3 << lBd2 << "(" << ((lBd2 - lDecd2) % lBd2) << "):" << sCadenaB64b << "::" << lBd4 << "(" << ((lBd4 - lDecd4) % lBd4) << "):" << sCadenaB64c;
		std::string stringx3 = stringstream3.str();

		return stringx3;
	}

	return "Aborted";
}

std::string createHashMulti(std::string inputFile, uint64_t tama, int sizeVentana, int bloque, int dec, int threads, bool debug, uint64_t tambuf)
{
	uint64_t lenA = 0, lenB = 0, lenC = 0;

	int64_t lBd1 = 0, lBd2 = 0, lBd4 = 0;
	int64_t lMd1 = 0, lMd2 = 0, lMd4 = 0;
	int64_t lDec = dec, lDecd1 = 0, lDecd2 = 0, lDecd4 = 0;	
	int64_t numChars = 64;

	bool Bd1m3 = false, Bd2m3 = false, Bd4m3 = false;

	size_t inFileSize = tama;
	int64_t delta = inFileSize/threads;

	int lB = bloque;

	if (lB <= 0)
	{
		int cociente = std::ceil(((double)inFileSize) / 64);

		lB = 3;
		while (lB < cociente)
		{
			lB = lB * 2;
		}
	}

	lBd1 = lB;
	lBd2 = lB / 2;
	lBd4 = lB / 4;

	lDecd1 = lDec;
	lDecd2 = lDec;
	lDecd4 = lDec;

	if (lBd1 < 3)
	{
		Bd1m3 = true;
	}
	else
	{
		lMd1 = (lBd1 - lDecd1) % lBd1;
	}

	if (lBd2 < 3)
	{
		Bd2m3 = true;
	}
	else
	{
		lMd2 = (lBd2 - lDecd2) % lBd2;
	}

	if (lBd4 < 3)
	{
		Bd4m3 = true;
	}
	else
	{
		lMd4 = (lBd4 - lDecd4) % lBd4;
	}	


	while ((lenA < numChars) && (lenB < numChars) && !(Bd1m3&&Bd2m3&&Bd4m3))
	{	
		#pragma omp parallel num_threads(threads) shared(tambuf,Bd1m3,Bd2m3,Bd4m3,lMd1,lMd2,lMd4,lBd1,lBd2,lBd4,lDecd1,lDecd2,lDecd4,delta,inFileSize,bloque,inputFile,dec) 
		{
			std::string base64code = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			uint64_t inicio = 0, fin = 0;
			std::string sCadenaB64a, sCadenaB64b, sCadenaB64c;
			uint64_t num_thread = omp_get_thread_num();

			std::ifstream in(inputFile,std::ios::binary);

			if(num_thread != (threads-1))
			{
				inicio = num_thread*delta;
				fin = (((num_thread+1)*delta)-1);
			}
			else
			{
				inicio = num_thread*delta;
				fin = inFileSize-1;
			}		
			char bValor = 0;
			auto * baVentana = new char[sizeVentana];
			int64_t n = 0;

			for(int i=0;i<sizeVentana;i++)
			{
				baVentana[i]=0;
			}

			in.seekg(inicio);
			n = inicio % sizeVentana;			

			bool firstA = true, firstB = true, firstC = true;

			int64_t lAi = 0;

			int64_t h1 = 0, h2 = 0, h3 = 0, resultado = 0;
			int64_t hashPrime = 0x01000193;
			int64_t hashInit = 0x28021967;
			int64_t h22a = hashInit;
			int64_t h22b = hashInit;
			int64_t h22c = hashInit;
			int64_t lBfb = 0;
			int64_t lHia = 0;
			int64_t lHib = 0;
			int64_t lHic = 0;
			int64_t lDecfb = 0;
			int64_t temp = 0;
			bool noCadena = false;

			auto* memblock = new char[tambuf];
			uint64_t memblocksize = tambuf;

			

			bool acabado = false, finA = false, finB = false, finC = false;	   

			uint64_t pos = inicio;
			uint64_t len = memblocksize;
			uint64_t vuelta = 0;

		    for (; !acabado; vuelta++)
		    {
				
		    	if((inFileSize-pos)<len)
		        {
		        	len = (inFileSize-pos);
		        	acabado = true;
		        }

		        in.read(&memblock[0], len);

				for (uint64_t index = 0; index < len; index++)
				{
					bValor = memblock[index];
					pos++;

					h2 = h2 - h1;
					h2 = h2 + sizeVentana * (bValor & 0x00FF);
					h2 = h2 & 0x00FFFFFFFF;

					h1 = h1 + (bValor & 0x00FF);
					h1 = h1 - (baVentana[n] & 0x00FF);
					h1 = h1 & 0x00FFFFFFFF;

					baVentana[n] = bValor;
					n = (++n) % sizeVentana;

					h3 = (h3 << 5) & 0x00FFFFFFFF;
					h3 = h3^(bValor&0x00FF);
					h3 = h3 & 0x00FFFFFFFF;

					resultado = (h1 + h2 + h3) & 0x00FFFFFFFF;
					lAi = resultado;

					// MODIFIED FNV COMPUTATION

					if((num_thread==0)||(!firstA))
					{
						h22a = 0xFFFFFFFF & (h22a*hashPrime);
						h22a = 0xFFFFFFFF & (h22a ^ (0xFF & bValor));
					}

					if((num_thread==0)||(!firstB))
					{					
						h22b = 0xFFFFFFFF & (h22b*hashPrime);
						h22b = 0xFFFFFFFF & (h22b ^ (0xFF & bValor));
					}

					if((num_thread==0)||(!firstC))
					{
						h22c = 0xFFFFFFFF & (h22c*hashPrime);
						h22c = 0xFFFFFFFF & (h22c ^ (0xFF & bValor));
					}

					if((vuelta>0)||(index > sizeVentana)) 
					{
						if (!Bd4m3)
						{
							// If /4 is trigger point
							if ((((lAi%lBd4) == lMd4)))
							{
								if(!finC)
								{
									lHic = h22c;
									h22c = hashInit;
												
									if((num_thread==0)||(!firstC))
									{
										// Second character
										temp = (int)((lHic >> 6) % 64);
										sCadenaB64c = sCadenaB64c.append(base64code.substr(temp, 1));
																	
										// First character
										temp = (int)(lHic % 64);
										sCadenaB64c = sCadenaB64c.append(base64code.substr(temp, 1));
									}
									else
									{
										firstC = false;
									}							
								}

								if(pos > (fin+sizeVentana))
								{
									finC = true;
								}						

								// If /2 is trigger point
								if (((!Bd2m3) && (lAi%lBd2) == lMd2))
								{
									lHib = h22b;
									h22b = hashInit;

									if(!finB)
									{
										if((num_thread==0)||(!firstB))
										{
											// Second character
											temp = (int)((lHib >> 6) % 64);
											sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));

											// First character
											temp = (int)(lHib % 64);
											sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));
										}
										else
										{
											firstB = false;
										}
									}

									if(pos > (fin+sizeVentana))
									{
										finB = true;
									}							

									// If /1 is trigger point
									if (((!Bd1m3) && ((lAi%lBd1) == lMd1)))
									{
										if(!finA)
										{
											lHia = h22a;
											h22a = hashInit;

											if((num_thread==0)||(!firstA))
											{
												// Second character
												temp = (int)((lHia >> 6) % 64);
												sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
																					
												// First character
												temp = (int)(lHia % 64);
												sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
											}	
											else
											{
												firstA = false;
											}
										}

										if(pos > (fin+sizeVentana))
										{
											finA = true;
										}								
									} 
								} 
							} 
						} 
						else
						{
							if (!Bd2m3)
							{
								// If /2 is trigger point
								if (((lAi%lBd2) == lMd2))
								{
									if(!finB)
									{
										lHib = h22b;
										h22b = hashInit;

										if((num_thread==0)||(!firstB))
										{
											// Second character
											temp = (int)((lHib >> 6) % 64);
											sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));
																			
											// First character
											temp = (int)(lHib % 64);
											sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));
										}
										else
										{
											firstB = false;
										}
									}

									if(pos > (fin+sizeVentana))
									{
										finB = true;
									}							

									// If /1 is trigger point
									if (((!Bd1m3) && ((lAi%lBd1) == lMd1)))
									{
										if(!finA)
										{
											if((num_thread==0)||(!firstA))
											{
												lHia = h22a;
												h22a = hashInit;
																					
												// Second character
												temp = (int)((lHia >> 6) % 64);
												sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));

												// First character
												temp = (int)(lHia % 64);
												sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));												
											}
											else
											{
												firstA = false;
											}	
										}

										if(pos > (fin+sizeVentana))
										{
											finA = true;
										}								

									} 
								}                  
							} 
							else
							{
								if (!Bd1m3)
								{
									// If /1 is trigger point
									if (((lAi%lBd1) == lMd1))
									{
										if(!finA)
										{
											lHia = h22a;
											h22a = hashInit;
												
											if((num_thread==0)||(!firstA))
											{
												// Second character
												temp = (int)((lHia >> 6) % 64);
												sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));

												// First character
												temp = (int)(lHia % 64);
												sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
											}
											else
											{
												firstA = false;
											}
										}

										if(pos > (fin+sizeVentana))
										{
											finA = true;
										}								
									}                                  
								}
							}
						} 
					}
				}

				if((finA&&finB&&finC)||((num_thread==threads-1)&&(pos>=fin)))
				{
					acabado = true;
				}
			} 

			// Last pair
			if (num_thread==(threads-1))
			{
				lHia = h22a;
				
				// Second character
				temp = (int)((lHia >> 6) % 64);
				sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));
				
				// First character
				temp = (int)(lHia % 64);
				sCadenaB64a = sCadenaB64a.append(base64code.substr(temp, 1));

				lHib = h22b;
				h22b = hashInit;
				
				// Second character
				temp = (int)((lHib >> 6) % 64);
				sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));
				
				// First character
				temp = (int)(lHib % 64);
				sCadenaB64b = sCadenaB64b.append(base64code.substr(temp, 1));

				lHic = h22c;
				h22c = hashInit;
				
				// Second character
				temp = (int)((lHic >> 6) % 64);
				sCadenaB64c = sCadenaB64c.append(base64code.substr(temp, 1));
				
				// First character
				temp = (int)(lHic % 64);
				sCadenaB64c = sCadenaB64c.append(base64code.substr(temp, 1));
			}

			listOfHashesA[num_thread] = sCadenaB64a;
			listOfHashesB[num_thread] = sCadenaB64b;
			listOfHashesC[num_thread] = sCadenaB64c;

			in.close();

		} 

		lenA = 0;
		lenB = 0;
		lenC = 0;		

		for(int iii=0; iii<threads; iii++)
		{
			lenA = lenA + listOfHashesA[iii].size();
			lenB = lenB + listOfHashesB[iii].size();
			lenC = lenC + listOfHashesC[iii].size();				
		}

		if((lenA>=64)&&(lenB>=64))
		{
			std::stringstream stringstream5;

			stringstream5 << lBd1 << "(" << ((lBd1-lDecd1)%lBd1) << "):";

			for(int iii=0; iii<threads; iii++)
			{
				stringstream5 << listOfHashesA[iii];
			}

			stringstream5 << "::" << lBd2 << "(" << ((lBd2-lDecd2)%lBd2) << "):";

			for(int iii=0; iii<threads; iii++)
			{
				stringstream5 << listOfHashesB[iii];
			}

			std::string stringx5 = stringstream5.str();

			return stringx5;
		}
		else
		{
			if((lenB>=64)&&(lenC>=64))
			{
				std::stringstream stringstream6;

				stringstream6 << lBd2 << "(" << ((lBd2-lDecd2)%lBd2) << "):"; 
					
				for(int iii=0; iii<threads; iii++)
				{
					stringstream6 << listOfHashesB[iii];
				}

				stringstream6 << "::" << lBd4 << "(" << ((lBd4-lDecd4)%lBd4) << "):";

				for(int iii=0; iii<threads; iii++)
				{
					stringstream6 << listOfHashesC[iii];
				}

				std::string stringx6 = stringstream6.str();

				return stringx6;
			}
			else
			{
				if(lenC>=64)
				{
					lBd1 = lBd4;
				}
				else
				{
					lBd1 = lBd4/2;
				}
				lBd2 = lBd1 / 2;
				lBd4 = lBd2 / 2;

				if((lBd4<3)&&(lBd2<3))
				{
					return "Aborted";
				}

				if (lBd1 < 3)
				{
					Bd1m3 = true;
				}
				else
				{
					lMd1 = (lBd1 - lDecd1) % lBd1;
				}

				if (lBd2 < 3)
				{
					Bd2m3 = true;
				}
				else
				{
					lMd2 = (lBd2 - lDecd2) % lBd2;
				}

				if (lBd4 < 3)
				{
					Bd4m3 = true;
				}
				else
				{
					lMd4 = (lBd4 - lDecd4) % lBd4;
				}
			}
		}
	}	

	return "Aborted";
}

std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace) 
{
    size_t pos = 0;

    while ((pos = subject.find(search, pos)) != std::string::npos) 
    {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }

    return subject;
}


float compareHashesDet2(char* X, char* Y, int m, int n, int** arrw, int* arrx, int* arry, std::vector<item> vItem, std::vector<item>::reverse_iterator iterador) 
{ 
	vItem.clear();

	for (int i = 0; i <= m; i++) 
    { 
    	arrx[i] = 0;

    	for (int j = 0; j <= n; j++) 
        { 
            if (i == 0 || j == 0)
            { 
                arrw[i][j] = 0;
                arry[j]=0; 
  			}
            else
            { 
            	arrw[i][j] = 0;

            	if (X[i - 1] == Y[j - 1]) 
            	{ 
            		if(arrw[i-1][j-1]==0) 
            		{
            			if((i%2==1)&&(j%2==1)) 
            			{
            				if(X[i] == Y[j]) 
            				{
	            				arrw[i][j] = 1;
	            			}
            			}

            		}
            		else 
            		{
            			if((i%2==1)&&(j%2==1)) 
            			{
            				if(X[i] == Y[j])
            				{
            					arrw[i][j] = arrw[i - 1][j - 1] + 1;
            				}
            			}
            			else
            			{
            				arrw[i][j] = arrw[i - 1][j - 1] + 1;

            				if((i<m)&&(j<n))
            				{
            					if(X[i]!=Y[j])
								{
									vItem.push_back(make_item(arrw[i][j],i,j));
								}
            				}
            				else 
            				{
            					vItem.push_back(make_item(arrw[i][j],i,j));
            				}
            			}
            		}
            	}
            }
        } 
    } 

	float points = 0;

	for (iterador = vItem.rbegin(); iterador != vItem.rend(); ++iterador)
	{
		int iii;
		int jjj;

		if((arrx[iterador->posRow]==0)&&(arry[iterador->posCol]==0))
		{

			points = points + iterador->val;

			for(iii = iterador->posRow - iterador->val+1,jjj = iterador->posCol - iterador->val+1; iii <= iterador->posRow, jjj <= iterador->posCol; iii++, jjj++)
			{
				if((arrx[iii] == 1)||(arry[jjj] == 1))
				{
					points--;
				}

				arrx[iii] = 1;
				arry[jjj] = 1;
			}
		}
		else 
		{
			int reducido = iterador->val;
			int quitado = 0;

			for(iii = iterador->posRow, jjj = iterador->posCol; iii >= iterador->posRow - iterador->val+1, jjj >= iterador->posCol - iterador->val+1; iii--,jjj--)
			{
				if((arrx[iii] == 1)||(arry[jjj] == 1))
				{
					reducido--;
					quitado++;
				}
				else
				{
					break;
				}
			}

			if(reducido>0)
			{
				points = points + reducido;

				for(iii = iterador->posRow - (reducido +quitado) +1, jjj = iterador->posCol - (reducido+quitado) +1; iii <= iterador->posRow - quitado, jjj <= iterador->posCol - quitado; iii++,jjj++)
				{
					if((arrx[iii] == 1)||(arry[jjj] == 1))
					{
						points--;
					}

					arrx[iii] = 1;
					arry[jjj] = 1;
				}
			}
		}
	}

	points = points/2;

	return (int)((points*100)/(std::max(m,n)/2));

}


float compareHashes(std::string string1, std::string string2, int** arrz, int* arrx, int* arry, std::vector<item> vItem, std::vector<item>::reverse_iterator iter, bool onlyfirst, int sizeArr)
{

	uint32_t blockSize1a=0, blockSize1b = 0, blockSize2a = 0, blockSize2b = 0;
	uint32_t dec1a = 0, dec1b = 0, dec2a = 0, dec2b = 0;
	int resultado = 0;

	float res1 = 0, res2 = 0, resD = 0, resI = 0, resF = 0;

	std::string sig1a = "", sig1b = "", sig2a = "", sig2b = ""; 

	///////////////////////////////////////////////////////////
	// SIGNATURE RETRIEVAL

	// File 1

	blockSize1a = std::stoi(string1.substr(0,string1.find("(")));
	string1 = string1.substr(string1.find("(")+1);
	dec1a = std::stoi(string1.substr(0,string1.find(")")));
	string1 = string1.substr(string1.find(":")+1);
	sig1a = string1.substr(0,string1.find("::"));
	string1 = string1.substr(string1.find("::")+2);
	blockSize1b = std::stoi(string1.substr(0,string1.find("(")));
	string1 = string1.substr(string1.find("(")+1);
	dec1b = std::stoi(string1.substr(0,string1.find(")")));
	sig1b = string1.substr(string1.find(":")+1);

	// File 2

	blockSize2a = std::stoi(string2.substr(0,string2.find("(")));
	string2 = string2.substr(string2.find("(")+1);
	dec2a = std::stoi(string2.substr(0,string2.find(")")));
	string2 = string2.substr(string2.find(":")+1);
	sig2a = string2.substr(0,string2.find("::"));
	string2 = string2.substr(string2.find("::")+2);
	blockSize2b = std::stoi(string2.substr(0,string2.find("(")));
	string2 = string2.substr(string2.find("(")+1);
	dec2b = std::stoi(string2.substr(0,string2.find(")")));
	sig2b = string2.substr(string2.find(":")+1);

	if(sig1a.size()>sizeArr)
	{
		sig1a = sig1a.substr(0,sizeArr);
	}

	if(sig1b.size()>sizeArr)
	{
		sig1b = sig1b.substr(0,sizeArr);
	}

	if(sig2a.size()>sizeArr)
	{
		sig2a = sig2a.substr(0,sizeArr);
	}

	if(sig2b.size()>sizeArr)
	{
		sig2b = sig2b.substr(0,sizeArr);
	}

	if((blockSize1a == blockSize2a) && (dec1a == dec2a))
	{
		if(!((((sig1a.size()>sig2a.size()))&&(sig1a.size()>30*sig2a.size())) ||
			(((sig2a.size()>sig1a.size()))&&(sig2a.size()>30*sig1a.size()))))
		{

			char *X1 = const_cast<char*>(sig1a.c_str());
  			char *Y1 = const_cast<char*>(sig2a.c_str());

  			res2 = compareHashesDet2(X1,Y1,strlen(X1),strlen(Y1),arrz,arrx,arry,vItem,iter);

  			if(res2>res1)
  			{
  				res1 = res2;
  			}
		}

	}

	if((blockSize1b == blockSize2b) && (dec1b == dec2b) && !onlyfirst)
	{
		if(!((((sig1b.size()>sig2b.size()))&&(sig1b.size()>30*sig2b.size())) ||
			(((sig2b.size()>sig1b.size()))&&(sig2b.size()>30*sig1b.size()))))
		{

			char *X1 = const_cast<char*>(sig1b.c_str());
  			char *Y1 = const_cast<char*>(sig2b.c_str());

  			res2 = compareHashesDet2(X1,Y1,strlen(X1),strlen(Y1),arrz,arrx,arry,vItem,iter);

  			if(res2>res1)
  			{
  				res1 = res2;
  			}
		}

	}


	if((blockSize1a == blockSize2b) && (dec1a == dec2b))
	{
		if(!((((sig1a.size()>sig2b.size()))&&(sig1a.size()>30*sig2b.size())) ||
			(((sig2b.size()>sig1a.size()))&&(sig2b.size()>30*sig1a.size()))))
		{

			char *X1 = const_cast<char*>(sig1a.c_str());
  			char *Y1 = const_cast<char*>(sig2b.c_str());
  			res2 = compareHashesDet2(X1,Y1,strlen(X1),strlen(Y1),arrz,arrx,arry,vItem,iter);

  			if(res2>res1)
  			{
  				res1 = res2;
  			}
		}

	}


	if((blockSize1b == blockSize2a) && (dec1b == dec2a))
	{
		if(!((((sig1b.size()>sig2a.size()))&&(sig1b.size()>30*sig2a.size())) ||
			(((sig2a.size()>sig1b.size()))&&(sig2a.size()>30*sig1b.size()))))
		{

			char *X1 = const_cast<char*>(sig1b.c_str());
  			char *Y1 = const_cast<char*>(sig2a.c_str());
  			res2 = compareHashesDet2(X1,Y1,strlen(X1),strlen(Y1),arrz,arrx,arry,vItem,iter);

  			if(res2>res1)
  			{
  				res1 = res2;
  			}
		}

	}	

	return res1;
}



int main(int argc, char* argv[])
{

#ifdef __unix__    
   OS=0;
#endif

#ifdef linux    
   OS=1;
#endif

#ifdef _WIN32
   OS=2;
#endif

#ifdef _WIN64
   OS=3;
#endif

	int numer = 1;
	int w = 7;
	int b = 0;
	int d = 1;
	int t = 0;

	int i = 0;	

	bool reconocido = false;
	bool boolI1 = false;
	bool boolI2 = false;
	bool boolO = false;
	bool boolV = false;
	bool boolB = false;
	bool boolD = false;
	bool boolT = false;	
	bool boolH = false;
	bool boolC = false;
	bool boolX = false;
	bool boolR = false;
	bool boolHash1 = false;
	bool boolHash2 = false;
	bool debug = false;
	bool boolList = false;

	std::string sTemp;
	std::string sFilenameInput1;
	std::string sFilenameInput2;
	std::string sFilenameOutput;
	std::string sSiSe = "SiSe", sTemp1 = "", sTemp2 = "", sFileX = "";
	std::string str1 = "", str2 = "";

	size_t sizeF1=0, sizeF2=0, tamabuf=2, fileSize;
	uint64_t onem=1048576, comp=0;
	float result=0;

	fs::path pF1, pF2;

	std::vector<std::string> filesLess1MB; 
	std::vector<std::string> filesMore1MB; 
	std::vector<std::string>::iterator it;

	unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
	int num_cores_available = std::thread::hardware_concurrency();
	t = num_cores_available;

	if(argc == 1)
	{
		printFormat();
		return 1;
	}

	sTemp = argv[1];

	if((sTemp.substr(0,1)).compare("-")==0) 
	{
		while (numer < argc)
		{
			reconocido = false;
			sTemp = argv[numer];


			if (!sTemp.compare("-x")) 
			{
				if (++numer < argc)
				{
					boolX = true;
					sFilenameInput1 = argv[numer++];
					reconocido = true;
				}
			}

			if (!sTemp.compare("-r")) 
			{
				if (++numer < argc)
				{
					boolR = true;
					sFilenameInput1 = argv[numer++];
					reconocido = true;
				}
			}

			if (!sTemp.compare("-i")) 
			{
				if (++numer < argc)
				{
					boolI1 = true;
					sFilenameInput1 = argv[numer++];
					reconocido = true;

					if ((numer < argc) && (((std::string)argv[numer]).find("-", 0) != 0))
					{
						boolI2 = true;
						sFilenameInput2 = argv[numer++];
					}

				}
				else
				{
					printFormat();
					return 0;
				}
			}

			//Compare two elements (either files or hashes)
			if (!sTemp.compare("-c")) 
			{
				if (++numer < argc)
				{
					boolI1 = true;
					sFilenameInput1 = argv[numer++];
					reconocido = true;

					if ((numer < argc) && (((std::string)argv[numer]).find("-", 0) != 0))
					{
						boolI2 = true;
						sFilenameInput2 = argv[numer++];
						boolC = true; 
					}
				}
				else
				{
					printFormat();
					return 0;
				}
			}			

			if (!sTemp.compare("-o"))
			{
				if (++numer < argc)
				{
					boolO = true;
					sFilenameOutput = argv[numer++];
					reconocido = true;
				}
				else
				{
					printFormat();
					return 0;
				}
			}

			if (!sTemp.compare("-v") || !sTemp.compare("-w"))
			{
				if (++numer < argc)
				{
					boolV = true;
					std::istringstream reader(argv[numer++]);
					reader >> w;
					reconocido = true;
				}
				else
				{
					printFormat();
					return 0;
				}
			}

			if (!sTemp.compare("-b"))
			{
				if (++numer < argc)
				{
					boolB = true;
					std::istringstream reader(argv[numer++]);
					reader >> b;
					reconocido = true;
				}
				else
				{
					printFormat();
					return 0;
				}
			}

			if (!sTemp.compare("-d"))
			{
				if (++numer < argc)
				{
					boolD = true;
					std::istringstream reader(argv[numer++]);
					reader >> d;
					reconocido = true;
				}
				else
				{
					printFormat();
					return 0;
				}
			}

			if (!sTemp.compare("-t"))
			{
				if (++numer < argc)
				{
					boolT = true;
					std::istringstream reader(argv[numer++]);
					reader >> t;
					reconocido = true;
				}
				else
				{
					printFormat();
					return 0;
				}
			}		

			if (!sTemp.compare("-debug"))
			{
				debug = true;
				numer++;
				reconocido = true;
			}

			if (!reconocido)
			{
				numer++;
			}

		}
	}
	else // File, list of files or folder
	{
		boolList = true;

		for(int i=1; i < argc; i++)
		{
			fileSize = fs::file_size(argv[i]);
			
			if(fileSize < 1048576*14)
			{
				filesLess1MB.push_back(argv[i]);
			}
			else
			{
				filesMore1MB.push_back(argv[i]);
			}

		}

		std::cout << "SiSe-1.0--" << w << ":" << d << "--blocksize:hash,filename" << std::endl;

		int th = 0;
		if(filesLess1MB.size() > 1)
		{
			////////////////////////////////////////		
			// Multi.threading for small files

			if(filesLess1MB.size() < num_cores_available)
			{
				th = filesLess1MB.size();
			}
			else
			{
				th = num_cores_available;
			}

			bool semaphor = true;
			size_t numberFilesLess1MB = filesLess1MB.size();

			#pragma omp parallel num_threads(th) shared(filesLess1MB,semaphor,t,numberFilesLess1MB) 
			{
				uint64_t num_thread = omp_get_thread_num();
				uint32_t counterInVector = num_thread;
				size_t thisFileSize, thisTamaBuf;
				uint64_t thisComp=0;
				std::string thisTemp = "";
				fs::path thisP;
            
				while(counterInVector < numberFilesLess1MB)
				{
					thisFileSize = fs::file_size(filesLess1MB[counterInVector]);
					
					thisP = filesLess1MB[counterInVector];

					tamabuf= std::min(tamabuf,onem);

					thisTemp = createHash(filesLess1MB[counterInVector], thisFileSize, w, b, d, debug, thisFileSize);

					while(!semaphor)
				    {
				    	std::this_thread::sleep_for(std::chrono::milliseconds(num_thread*5));
				    }

				    semaphor = false;

					std::cout << thisTemp << ",\"" << ReplaceString((fs::absolute(thisP)).u8string(),"\\\\","\\")<<"\""<< std::endl;

				    semaphor = true;

					counterInVector = counterInVector + t;
				}
			}
		}

		if(filesLess1MB.size() == 1)
		{
			filesMore1MB.push_back(filesLess1MB[0]);
		}

		///////////////////////////////////////			
		// Multi-threading for large files
			
		fs::path thisP;

		for(it = filesMore1MB.begin(); it != filesMore1MB.end(); it++,i++ )
		{
		    fileSize = 0;

		    if(fs::is_regular_file(*it))
		    {
				try
				{
					fileSize = fs::file_size(*it);
					thisP = *it;
				}
				catch(const fs::filesystem_error& err)
				{
					std::cerr << "File not found !!!" << std::endl << err.what() << std::endl;
					return 1;
				}

				tamabuf = 2;
				
				comp = fileSize/t;

				while(tamabuf < comp)
				{
					tamabuf = tamabuf*2;
				}

						
				tamabuf= std::min(tamabuf,onem);

			    if(t==1)
				{
					sTemp = createHash(*it, fileSize, w, b, d, debug,tamabuf);
				}
				else
				{
					listOfHashesA = new std::string[t]();
					listOfHashesB = new std::string[t]();
					listOfHashesC = new std::string[t]();

					sTemp = createHashMulti(*it, fileSize, w, b, d, t, debug, tamabuf);
				}

				if (sTemp.compare("Aborted: String too long")) // IF NOT TOO LONG
				{
			       	std::cout << sTemp << ",\"" << ReplaceString((fs::absolute(thisP)).u8string(),"\\\\","\\")<<"\""<< std::endl;
				}
			}
		}
	}

	boolH = boolI1 && (!boolI2);

	////////////////////////////////////////////
	// OPERATION WITH 1 FILE - CREATE HASH
	////////////////////////////////////////////  	

	if (boolI1 && (!boolI2))
	{
		size_t inFileSize3;

		try
		{
			inFileSize3 = fs::file_size(sFilenameInput1);
		}
		catch(const fs::filesystem_error& err)
		{
			if(((err.code().message()).compare("Is a directory"))==0)
			{
				boolI1 = false;
				boolR = true;
			}
			else
			{
				std::cerr << "File not found !!!" << std::endl << err.what() << std::endl;
				return 1;
			}
		}
	}

	if (boolI1 && (!boolI2)) 
	{

		size_t inFileSize3;

		try
		{
			inFileSize3 = fs::file_size(sFilenameInput1);
		}
		catch(const fs::filesystem_error& err)
		{
			std::cerr << "File not found !!!" << std::endl << err.what() << std::endl;
			return 1;
		}

		tamabuf = 2;

		comp = inFileSize3/t;

		while(tamabuf < comp)
		{
			tamabuf = tamabuf*2;
		}

		tamabuf= std::min(tamabuf,onem);

		if(t==1)
		{
			sTemp = createHash(sFilenameInput1, inFileSize3, w, b, d, debug,tamabuf);
		}
		else
		{
			listOfHashesA = new std::string[t]();
			listOfHashesB = new std::string[t]();
			listOfHashesC = new std::string[t]();

			sTemp = createHashMulti(sFilenameInput1, inFileSize3, w, b, d, t, debug, tamabuf);	
		}

		if (!sTemp.compare("Aborted: String too long"))
		{
			std::cout << "*** Operation aborted: the signature generated is out of bounds ***" << std::endl;
			std::cout << sTemp << std::endl;
			return 1;
		}
		else
		{
			std::stringstream stringstream2;
			stringstream2 << "SiSe-1.0--" << w << ":" << d << "--blocksize:hash,filename" << std::endl;
            fs::path p = sFilenameInput1;
            stringstream2 << sTemp << ",\"" << ReplaceString((fs::absolute(p)).u8string(),"\\\\","\\")<<"\"" << std::endl;
			std::cout << stringstream2.str();

			if (boolO)
			{
				std::ofstream myFile2(sFilenameOutput, std::ios::out | std::ios::binary);
				myFile2 << stringstream2.str();
				myFile2.close();
			}

			std::string stringx1 = stringstream2.str();
		}
	}

	if(boolR) // CREATE HASH OF ALL THE FILES IN A FOLDER
	{
		size_t inFileSize4 = 0;
		setlocale(LC_CTYPE, "");

		filesLess1MB.clear();
		filesMore1MB.clear();

		for (const auto & entry : fs::directory_iterator(sFilenameInput1))
		{
	        if(fs::is_regular_file(entry.path()))
	        {
				try
				{
					inFileSize4 = fs::file_size(entry.path());
			
					if(inFileSize4 < 1048576*14)
					{
						filesLess1MB.push_back((entry.path()).string());
					}
					else
					{
						filesMore1MB.push_back((entry.path()).string());
					}

				}
				catch(const fs::filesystem_error& err)
				{
					return 1;
				}
			}
		}

		std::cout << "SiSe-1.0--" << w << ":" << d << "--blocksize:hash,filename" << std::endl;

		int th = 0;

		if(filesLess1MB.size() > 1)
		{
			////////////////////////////////////////		
			// Multi-threading for small files

			if(filesLess1MB.size() < num_cores_available)
			{
				th = filesLess1MB.size();
			}
			else
			{
				th = num_cores_available;
			}

			bool semaphor = true;
			size_t numberFilesLess1MB = filesLess1MB.size();

			
			#pragma omp parallel num_threads(th) shared(filesLess1MB,semaphor,t,numberFilesLess1MB) 
			{
				uint64_t num_thread = omp_get_thread_num();
				uint32_t counterInVector = num_thread;
				size_t thisFileSize, thisTamaBuf;
				uint64_t thisComp=0;
				std::string thisTemp = "";
				fs::path thisP;
            
				while(counterInVector < numberFilesLess1MB)
				{
					thisFileSize = fs::file_size(filesLess1MB[counterInVector]);
					
					thisP = filesLess1MB[counterInVector];

					tamabuf= std::min(tamabuf,onem);

					thisTemp = createHash(filesLess1MB[counterInVector], thisFileSize, w, b, d, debug, thisFileSize);

					while(!semaphor)
				    {
				    	std::this_thread::sleep_for(std::chrono::milliseconds(num_thread*5));
				    }

				    semaphor = false;

					std::cout << thisTemp << ",\"" << ReplaceString((fs::absolute(thisP)).u8string(),"\\\\","\\")<<"\""<< std::endl;

				    semaphor = true;

					counterInVector = counterInVector + t;
				}
			}
		}

		if(filesLess1MB.size() == 1)
		{
			filesMore1MB.push_back(filesLess1MB[0]);
		}

		///////////////////////////////////////			
		// Multi-threading for large files

		fs::path thisP;

		for(it = filesMore1MB.begin(); it != filesMore1MB.end(); it++,i++ )
		{
		    fileSize = 0;

		    if(fs::is_regular_file(*it))
		    {
				try
				{
					fileSize = fs::file_size(*it);
					thisP = *it;
				}
				catch(const fs::filesystem_error& err)
				{
					std::cerr << "File not found !!!" << std::endl << err.what() << std::endl;
					return 1;
				}


				tamabuf = 2;
				comp = fileSize/t;

				while(tamabuf < comp)
				{
					tamabuf = tamabuf*2;
				}

						
				tamabuf= std::min(tamabuf,onem);

			    if(t==1)
				{
					sTemp = createHash(*it, fileSize, w, b, d, debug,tamabuf);
				}
				else
				{
					listOfHashesA = new std::string[t]();
					listOfHashesB = new std::string[t]();
					listOfHashesC = new std::string[t]();

					sTemp = createHashMulti(*it, fileSize, w, b, d, t, debug, tamabuf);
				}

				if (sTemp.compare("Aborted: String too long")) // IF NOT TOO LONG
				{
			       	std::cout << sTemp << ",\"" << ReplaceString((fs::absolute(thisP)).u8string(),"\\\\","\\")<<"\""<< std::endl;
				}
			}
		}
	}

	////////////////////////////////////////////
	// OPERATIONS WITH 2 FILES
	////////////////////////////////////////////  	

	if(boolC)
	{

		int** miarr22 = new int*[10*512+1];
		    
		for(int i = 0; i < 10*512+1; i++)
		{
			miarr22[i] = new int[10*512+1];
		} 

		int * miarrx22 = new int[10*512+1];
		int * miarry22 = new int[10*512+1];
		std::vector<item> vItem22;
		std::vector<item>::reverse_iterator iter22;


		if (boolI1 && boolI2 && (sFilenameInput1.size()>1) && (sFilenameInput2.size()>1))
		{

			try
			{
				sizeF1 = fs::file_size(sFilenameInput1);
			}
			catch(const fs::filesystem_error& err)
			{
				std::cerr << "File not found !!!" << std::endl << err.what() << std::endl;
				return 1;
			}


			str1 = "";

			if(sizeF1 < 10000)
			{
				std::ifstream inF1(sFilenameInput1);

			    if (!inF1.is_open())
			    {
			        std::cout << "Can't open input file: " << sFilenameInput1 << std::endl;
			        return 1;
			    }

			    str1 = static_cast<std::stringstream const&>(std::stringstream() << inF1.rdbuf()).str();
			}
			else
			{
				str1 = "";
			}

		    if(str1.find(sSiSe)==0)
		    {
		    	boolHash1 = true;
		    }


			try
			{
				sizeF2 = fs::file_size(sFilenameInput2);
			}
			catch(const fs::filesystem_error& err)
			{
				std::cerr << "File not found !!!" << std::endl << err.what() << std::endl;
				return 1;
			}

			str2 = "";

			if(sizeF2 < 10000)
			{

				std::ifstream inF2(sFilenameInput2);

			    if (!inF2.is_open())
			    {
			        std::cout << "Can't open input file: " << sFilenameInput2 << std::endl;
			        return 1;
			    }

			    str2 = static_cast<std::stringstream const&>(std::stringstream() << inF2.rdbuf()).str();
			}
			else
			{
				str2 = "";
			}

		    if(str2.find(sSiSe)==0)
		    {
		    	boolHash2 = true;
		    }

		    sTemp1 = "";
		    sTemp2 = "";

		    // HASHES COMPARISON
		    if(boolHash1 && boolHash2) 
		    {
		    	sTemp1 = str1.substr(0,str1.find("\n"));
		    	sTemp2 = str2.substr(0,str2.find("\n"));
		    	
		    	if (sTemp1.compare(sTemp2) != 0)
		    	{
		    		std::cout << "ERROR: header information not compatible" << std::endl;
		    		return 1;
		    	}

		    	str1 = str1.substr(str1.find("\n")+1);
		    	str2 = str2.substr(str2.find("\n")+1);

		    	str1 = str1.substr(0,str1.find(","));
		    	str2 = str2.substr(0,str2.find(","));

		    	result = compareHashes(str1,str2,miarr22,miarrx22,miarry22,vItem22,iter22,false,10*512);

		    	if(result < 0)
		    	{
		    		std::cout << "RESULT: - (block sizes not compatible)" << std::endl;
		    		return 1;
		    	}

		    	pF1 = sFilenameInput1;
	            std::cout << ReplaceString((fs::absolute(pF1)).u8string(),"\\\\","\\") <<" matches " ;
						
				pF2 = sFilenameInput2;
				std::cout << ReplaceString((fs::absolute(pF2)).u8string(),"\\\\","\\") <<" (";
				std::cout << result << ")" << std::endl;
		    	return 0;
		    }
		    else
		    {
		    	// COMPARISON OF HASH AND FILE
		    	if((boolHash1&&!boolHash2)||((!boolHash1&&boolHash2)))
		    	{
		    		// HASH IN FILE 1
		    		if(boolHash1)
		    		{
		    			str1 = str1.substr(str1.find("\n")+1);
				    	str1 = str1.substr(0,str1.find(","));

				    	b = std::stoi(str1.substr(0,str1.find("(")));
				    	d = 1;
				    	sizeF2 = fs::file_size(sFilenameInput2);

						tamabuf = 2;

						if(sizeF2 < 1048576*14)
						{
							t=1;
						}
						else
						{
							t=num_cores_available;
						}

						comp = sizeF2/t;

						while(tamabuf < comp)
						{
							tamabuf = tamabuf*2;
						}

						
						tamabuf= std::min(tamabuf,onem);

				    	str2 = createHash(sFilenameInput2, sizeF2, w, b, d, debug, tamabuf);	

				    	result = compareHashes(str1,str2,miarr22,miarrx22,miarry22,vItem22,iter22,false,10*512);

				    	if(result < 0)
				    	{
				    		std::cout << "RESULT: - (block sizes not compatible)" << std::endl;
				    		return 1;
				    	}

						pF1 = sFilenameInput1;
	            		std::cout << ReplaceString((fs::absolute(pF1)).u8string(),"\\\\","\\") <<" matches " ;
						
						pF2 = sFilenameInput2;
						std::cout << ReplaceString((fs::absolute(pF2)).u8string(),"\\\\","\\") <<" (";
				    	std::cout << result << ")" << std::endl;
				    	return 0;

		    		}
		    		// HASH IN FILE 2
		    		else
		    		{
						str2 = str2.substr(str2.find("\n")+1);
				    	str2 = str2.substr(0,str2.find(","));

				    	b = std::stoi(str2.substr(0,str2.find("(")));
				    	d = 1;
				    	sizeF1 = fs::file_size(sFilenameInput1);

						tamabuf = 2;

						if(sizeF1 < 1048576*14)
						{
							t=1;
						}
						else
						{
							t=num_cores_available;
						}

						comp = sizeF1/t;

						while(tamabuf < comp)
						{
							tamabuf = tamabuf*2;
						}

						
						tamabuf= std::min(tamabuf,onem);

				    	str1 = createHash(sFilenameInput1, sizeF1, w, b, d, debug, tamabuf);	

				    	result = compareHashes(str1,str2,miarr22,miarrx22,miarry22,vItem22,iter22,false,10*512); 

				    	if(result < 0)
				    	{
				    		std::cout << "RESULT: - (block sizes not compatible)" << std::endl;
				    		return 1;
				    	}

						pF1 = sFilenameInput1;
	            		std::cout << ReplaceString((fs::absolute(pF1)).u8string(),"\\\\","\\") <<" matches " ;
						
						pF2 = sFilenameInput2;
						std::cout << ReplaceString((fs::absolute(pF2)).u8string(),"\\\\","\\") <<" (";
				    	std::cout << result << ")" << std::endl;
				    	return 0;
		    		}
		    	}
		    	// COMPARISON OF FILE AND FILE
		    	else
		    	{

					tamabuf = 2;

					if(sizeF1 < sizeF2)
					{

						if(sizeF1 < 1048576*14)
						{
							t=1;
						}
						else
						{
							t=num_cores_available;
						}

						comp = sizeF1/t;

						while(tamabuf < comp)
						{
							tamabuf = tamabuf*2;
						}

						
						tamabuf= std::min(tamabuf,onem);

			    		str1 = createHash(sFilenameInput1, sizeF1, w, b, d, debug, tamabuf);

						b = std::stoi(str1.substr(0,str1.find("(")));
				    	d = 1;

				    	str2 = createHash(sFilenameInput2, sizeF2, w, b, d, debug, tamabuf);	

				    	result = compareHashes(str1,str2,miarr22,miarrx22,miarry22,vItem22,iter22,false,10*512);

				    	if(result < 0)
				    	{
				    		std::cout << "RESULT: - (block sizes not compatible)" << std::endl;
				    		return 1;
				    	}

				    	pF1 = sFilenameInput1;
	            		std::cout << ReplaceString((fs::absolute(pF1)).u8string(),"\\\\","\\") <<" matches " ;
						
						pF2 = sFilenameInput2;
						std::cout << ReplaceString((fs::absolute(pF2)).u8string(),"\\\\","\\") <<" (";
				    	std::cout << result << ")" << std::endl;

				    	return 0;
			    	}
			    	else
			    	{
			    		if(sizeF2 < 1048576*14)
						{
							t=1;
						}
						else
						{
							t=num_cores_available;
						}

						comp = sizeF2/t;

						while(tamabuf < comp)
						{
							tamabuf = tamabuf*2;
						}

						
						tamabuf= std::min(tamabuf,onem);

			    		str2 = createHash(sFilenameInput2, sizeF2, w, b, d, debug, tamabuf);
			    	
			    		b = std::stoi(str2.substr(0,str2.find("(")));
				    	d = 1;

				    	str1 = createHash(sFilenameInput1, sizeF1, w, b, d, debug, tamabuf);	
				    	result = compareHashes(str1,str2,miarr22,miarrx22,miarry22,vItem22,iter22,false,10*512);

				    	if(result < 0)
				    	{
				    		std::cout << "RESULT: - (block sizes not compatible)" << std::endl;
				    		return 1;
				    	}

				    	pF1 = sFilenameInput1;
	            		std::cout << ReplaceString((fs::absolute(pF1)).u8string(),"\\\\","\\") <<" matches " ;
						
						pF2 = sFilenameInput2;
						std::cout << ReplaceString((fs::absolute(pF2)).u8string(),"\\\\","\\") <<" (";
				    	std::cout << result << ")" << std::endl;

				    	return 0;
			    	}	
		    	}
		    }
		}
	} 

	// OPTION -X: all against all with file containing signatures
	if(boolX) 
	{
		try
		{
			sizeF1 = fs::file_size(sFilenameInput1);
		}
		catch(const fs::filesystem_error& err)
		{
			std::cerr << "File not found !!!" << std::endl << err.what() << std::endl;
			return 1;
		}
		
		std::ifstream inF1(sFilenameInput1);

		if (!inF1.is_open())
		{
		    std::cout << "Can't open input file: " << sFilenameInput1 << std::endl;
		    return 1;
		}

		std::string str1 = static_cast<std::stringstream const&>(std::stringstream() << inF1.rdbuf()).str();
		    
			
		if(str1.find(sSiSe)!=0)
	    {
	    	std::cout << "ERROR: incorrect header file" << std::endl;
            return 1;			
	    }
		
	    sFileX = sFilenameInput1;

	    std::vector<std::string> strVec;

    	str1 = str1.substr(str1.find("\n")+1);
    	int a1 = str1.size();
    	int a2 = 0;

	    while((str1.size()>5)&&((a1-a2)>0))
	    {
	    	a1 = str1.size();
	    	sTemp1 = str1.substr(0,str1.find("\n"));
	    	strVec.push_back(sTemp1);
			str1 = str1.substr(str1.find("\n")+1);
			a2 = str1.size();
	    }

	    int strVecSize = strVec.size();

		if(t==1)
		{

			int** miarr44 = new int*[10*128+1];
    		int* miarrx44 = new int[10*128+1];
    		int* miarry44 = new int[10*128+1];
    		std::vector<item> vItem44;
    		std::vector<item>::reverse_iterator iter44;

			for(int i = 0; i < 10*128+1; i++)
			{
				miarr44[i] = new int[10*128+1];
			} 

			for(int i=0; i < strVecSize;i++)
			{
				str1 = strVec[i];
				sFilenameInput1 = str1.substr(str1.find(",")+2,str1.size()-(str1.find(",")+2)-1);
				str1 = str1.substr(0,str1.find(","));

				for(int j=i+1; j<strVecSize;j++)
				{
					str2 = strVec[j];
					sFilenameInput2 = str2.substr(str2.find(",")+2,str2.size()-(str2.find(",")+2)-1);
					str2 = str2.substr(0,str2.find(","));

		    		result = compareHashes(str1,str2,miarr44,miarrx44,miarry44,vItem44,iter44,false,10*128);

		    		if(result > 5)
		    		{
		            	std::cout << sFileX << ":" << sFilenameInput1 <<" matches " 
	    	        	          << sFilenameInput2 << " (" << result << ")" << std::endl;
					}
				}
			}
		}
		else
		{

			#pragma omp parallel num_threads(t) shared(strVecSize,strVec,sFileX) 
			{
				uint64_t num_thread = omp_get_thread_num();
				float resultx = 0;

				int** miarr66 = new int*[10*128+1];
    			int* miarrx66 = new int[10*128+1];
    			int* miarry66 = new int[10*128+1];
    			std::vector<item> vItem66;
    			std::vector<item>::reverse_iterator iter66;				
    
				for(int i = 0; i < 10*128+1; i++)
				{
					miarr66[i] = new int[10*128+1];
				} 

				for(int i=num_thread; i < strVecSize;)
				{
					std::string str1x = strVec[i];
					std::string sFilenameInput1x = str1x.substr(str1x.find(",")+2,str1x.size()-(str1x.find(",")+2)-1);
					str1x = str1x.substr(0,str1x.find(","));

					for(int j=i+1; j<strVecSize;j++)
					{
						std::string str2x = strVec[j];
						std::string sFilenameInput2x = str2x.substr(str2x.find(",")+2,str2x.size()-(str2x.find(",")+2)-1);
						str2x = str2x.substr(0,str2x.find(","));

			    		resultx = compareHashes(str1x,str2x,miarr66,miarrx66,miarry66,vItem66,iter66,false,10*128);

			    		if(resultx > 5)
			    		{
			            	std::stringstream ss;
							ss << sFileX << ":" << sFilenameInput1x <<" matches " << sFilenameInput2x << " (" << resultx << ")" << std::endl;
		    	        	std::cout << ss.str();       
						}
					}

					i = i+t;
				}
			}
		}
	}

	return 0; 
}

