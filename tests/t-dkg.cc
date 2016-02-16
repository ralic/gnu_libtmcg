/*******************************************************************************
   This file is part of LibTMCG.

 Copyright (C) 2016  Heiko Stamer <HeikoStamer@gmx.net>

   LibTMCG is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   LibTMCG is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with LibTMCG; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*******************************************************************************/

#include <libTMCG.hh>

#ifdef FORKING

#include <sstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include "test_helper.h"
#include "pipestream.hh"

#undef NDEBUG
#define N 5
#define T 2

int pipefd[N][N][2], broadcast_pipefd[N][N][2];
pid_t pid[N];

void start_instance
	(std::istream& crs_in, size_t whoami, bool faulty)
{
	if ((pid[whoami] = fork()) < 0)
		perror("t-dkg (fork)");
	else
	{
		if (pid[whoami] == 0)
		{
			/* BEGIN child code: participant P_i */
			
			// create pipe streams between all players
			std::vector<ipipestream*> P_in, bP_in;
			std::vector<opipestream*> P_out, bP_out;
			for (size_t i = 0; i < N; i++)
			{
				P_in.push_back(new ipipestream(pipefd[i][whoami][0]));
				P_out.push_back(new opipestream(pipefd[whoami][i][1]));
				bP_in.push_back(new ipipestream(broadcast_pipefd[i][whoami][0]));
				bP_out.push_back(new opipestream(broadcast_pipefd[whoami][i][1]));
			}
			
			// create TMCG and VTMF instances
			start_clock();
			BarnettSmartVTMF_dlog *vtmf = new BarnettSmartVTMF_dlog(crs_in);
			if (!vtmf->CheckGroup())
			{
				std::cout << "P_" << whoami << ": " <<
					"Group G was not correctly generated!" << std::endl;
				exit(-1);
			}
			stop_clock();
			std::cout << "P_" << whoami << ": " << elapsed_time() << std::endl;
			
			// create and exchange VTMF keys
			start_clock();
			vtmf->KeyGenerationProtocol_GenerateKey();
			for (size_t i = 0; i < N; i++)
			{
				if (i != whoami)
					vtmf->KeyGenerationProtocol_PublishKey(*P_out[i]);
			}
			for (size_t i = 0; i < N; i++)
			{
				if (i != whoami)
				{
					if (!vtmf->KeyGenerationProtocol_UpdateKey(*P_in[i]))
					{
						std::cout << "P_" << whoami << ": " << "Public key of P_" <<
							i << " was not correctly generated!" << std::endl;
						exit(-1);
					}
				}
			}
			vtmf->KeyGenerationProtocol_Finalize();
			stop_clock();
			std::cout << "P_" << whoami << ": " << elapsed_time() << std::endl;

			// create an instance of DKG
			GennaroJareckiKrawczykRabinDKG *dkg;
			std::cout << "GennaroJareckiKrawczykRabinDKG(" << N << ", " << T << ", ...)" << std::endl;
			dkg = new GennaroJareckiKrawczykRabinDKG(N, T,
				vtmf->p, vtmf->q, vtmf->g, vtmf->h);
			assert(dkg->CheckGroup());

			// convert pipestreams to input/output streams
			std::vector<std::istream*> C_in, B_in;
			std::vector<std::ostream*> C_out, B_out;
			for (size_t i = 0; i < N; i++)
			{
				C_in.push_back(P_in[i]);
				B_in.push_back(bP_in[i]);
				C_out.push_back(P_out[i]);
				B_out.push_back(bP_out[i]);
			}

			// create a simple broadcast protocol
			iobroadcast *iob = new iobroadcast(N, T, whoami, B_in, B_out);
			
			// generating $x$ and extracting $y = g^x \bmod p$
			std::stringstream err_log;
			start_clock();
			std::cout << "P_" << whoami << ": dkg.Generate()" << std::endl;
			if (faulty)
				dkg->Generate(whoami, C_in, C_out, iob, err_log, true);
			else
				assert(dkg->Generate(whoami, C_in, C_out, iob, err_log));
			stop_clock();
			std::cout << "P_" << whoami << ": " << elapsed_time() << std::endl;
			sleep(3 * whoami + (mpz_wrandom_ui() % N));
			std::cout << "P_" << whoami << ": log follows " << std::endl << err_log.str();
			
			// release broadcast
			delete iob;

			// release DKG and VTMF instances
			delete dkg, delete vtmf;
			
			// release pipe streams (private channels)
			size_t numRead = 0, numWrite = 0;
			for (size_t i = 0; i < N; i++)
			{
				numRead += P_in[i]->get_numRead() + P_out[i]->get_numRead();
				numWrite += P_in[i]->get_numWrite() + P_out[i]->get_numWrite();
				delete P_in[i], delete P_out[i];
			}
			std::cout << "P_" << whoami << ": numRead = " << numRead <<
				" numWrite = " << numWrite << std::endl;

			// release pipe streams (broadcast channel)
			numRead = 0, numWrite = 0;
			for (size_t i = 0; i < N; i++)
			{
				numRead += bP_in[i]->get_numRead() + bP_out[i]->get_numRead();
				numWrite += bP_in[i]->get_numWrite() + bP_out[i]->get_numWrite();
				delete bP_in[i], delete bP_out[i];
			}
			std::cout << "P_" << whoami << ": broadcast_numRead = " << numRead <<
				" broadcast_numWrite = " << numWrite << std::endl;
			
			std::cout << "P_" << whoami << ": exit(0)" << std::endl;
			exit(0);
			/* END child code: participant P_i */
		}
		else
			std::cout << "fork() = " << pid[whoami] << std::endl;
	}
}

int main
	(int argc, char **argv)
{
	assert(init_libTMCG());

	BarnettSmartVTMF_dlog 	*vtmf;
	std::stringstream 	crs;

	// create and check VTMF instance
	std::cout << "BarnettSmartVTMF_dlog()" << std::endl;
	vtmf = new BarnettSmartVTMF_dlog();
	std::cout << "vtmf.CheckGroup()" << std::endl;
	start_clock();
	assert(vtmf->CheckGroup());
	stop_clock();
	std::cout << elapsed_time() << std::endl;
	
	// publish VTMF instance as string stream (common reference string)
	std::cout << "vtmf.PublishGroup(crs)" << std::endl;
	vtmf->PublishGroup(crs);
	
	// open pipes
	for (size_t i = 0; i < N; i++)
		for (size_t j = 0; j < N; j++)
			if ((pipe(pipefd[i][j]) < 0) || (pipe(broadcast_pipefd[i][j]) < 0))
				perror("t-dkg (pipe)");
	
	// start childs (all correct)
	for (size_t i = 0; i < N; i++)
		start_instance(crs, i, false);
	
	// wait for childs and close pipes
	for (size_t i = 0; i < N; i++)
	{
		std::cerr << "waitpid(" << pid[i] << ")" << std::endl;
		if (waitpid(pid[i], NULL, 0) != pid[i])
			perror("t-dkg (waitpid)");
		for (size_t j = 0; j < N; j++)
		{
			if ((close(pipefd[i][j][0]) < 0) || (close(pipefd[i][j][1]) < 0))
				perror("t-dkg (close)");
			if ((close(broadcast_pipefd[i][j][0]) < 0) || (close(broadcast_pipefd[i][j][1]) < 0))
				perror("t-dkg (close)");
		}
	}

	// open pipes
	for (size_t i = 0; i < N; i++)
		for (size_t j = 0; j < N; j++)
			if ((pipe(pipefd[i][j]) < 0) || (pipe(broadcast_pipefd[i][j]) < 0))
				perror("t-dkg (pipe)");
	
	// start childs (two faulty parties)
	for (size_t i = 0; i < N; i++)
	{
		if ((i == (N - 1)) || (i == (N - 2)))
			start_instance(crs, i, true); // faulty
		else
			start_instance(crs, i, false);
	}
	
	// wait for childs and close pipes
	for (size_t i = 0; i < N; i++)
	{
		std::cerr << "waitpid(" << pid[i] << ")" << std::endl;
		if (waitpid(pid[i], NULL, 0) != pid[i])
			perror("t-dkg (waitpid)");
		for (size_t j = 0; j < N; j++)
		{
			if ((close(pipefd[i][j][0]) < 0) || (close(pipefd[i][j][1]) < 0))
				perror("t-dkg (close)");
			if ((close(broadcast_pipefd[i][j][0]) < 0) || (close(broadcast_pipefd[i][j][1]) < 0))
				perror("t-dkg (close)");
		}
	}
	
	// release VTMF instance
	delete vtmf;
	
	return 0;
}

#else

int main
	(int argc, char **argv)
{
	std::cout << "test skipped" << std::endl;
	return 77;
}

#endif