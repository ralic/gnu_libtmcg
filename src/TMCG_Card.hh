/*******************************************************************************
  A data structure for cards. This file is part of libTMCG.

 Copyright (C) 2004, 2005  Heiko Stamer <stamer@gaos.org>

   libTMCG is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   libTMCG is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with libTMCG; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*******************************************************************************/

#ifndef INCLUDED_TMCG_Card_HH
	#define INCLUDED_TMCG_Card_HH
	
	// config.h
	#ifdef HAVE_CONFIG_H
		#include "config.h"
	#endif
	
	// C++/STL header
	#include <cstdlib>
	#include <cassert>
	#include <string>
	#include <iostream>
	#include <vector>
	
	// GNU multiple precision library
	#include <gmp.h>
	
	#include "mpz_srandom.h"
	#include "parse_helper.hh"
	
/** @brief A data structure for cards.
    
    This struct represents a card in the original encoding scheme of
    Schindelhauer [Sch98]. The type of the card is shared among the
    players by the usage of quadratic residues resp. non-residues.
    Thus the security relies on the well known Quadratic Residuosity
    Assumption (QRA). */
struct TMCG_Card
{
	/** The @f$n\times m@f$-matrix @f$Z@f$ encodes the type of the card
	    in a shared manner. For each of the @f$n@f$ players there is a
	    separate row and for each of the @f$m@f$ bits in the binary
	    representation of the card type there is a column. The elements
	    of the matrix are numbers from @f$\mathbb{Z}^{\circ}_{m_i}@f$
	    where @f$m_i@f$ is the public modul of the @f$i@f$th player. */
	std::vector< std::vector<MP_INT> >			z;
	
	/** This constructor initalizes a card with a @f$1\times 1@f$-matrix.
	    Later the function TMCG_Card::resize can be used to enlarge the
	    representation of the card. */
	TMCG_Card
		();
	
	/** This constructor initalizes a card with a @f$n\times m@f$-matrix.
	    @param n is the number of players.
	    @param m is the number of bits used in the binary representation
	           of the card type. */
	TMCG_Card
		(size_t n, size_t m);
	
	/** A simple copy-constructor.
	    @param that is the card to be copied. */
	TMCG_Card
		(const TMCG_Card& that);
	
	/** A simple assignment-operator.
	    @param that is the card to be assigned. */
	TMCG_Card& operator =
		(const TMCG_Card& that);
	
	/** This operator tests two card representations for equality. */
	bool operator ==
		(const TMCG_Card& that) const;
	
	/** This operator tests two card representations for inequality. */
	bool operator !=
		(const TMCG_Card& that) const;
	
	/** This function resizes the representation of the card. The current
	    content will be released and a new @f$n\times m@f$-matrix created.
	    @param n is the number of players.
	    @param m is the number of bits used in the binary representation
	           of the card type. */
	void resize
		(size_t n, size_t m);
	
	/** This function imports the card.
	    @param s is correctly formated input string.
	    @returns True, if the import was successful. */
	bool import
		(std::string s);
	
	/** This destructor releases all occupied resources. */
	~TMCG_Card
		();
};

/** @relates TMCG_Card
    This operator prints a card to an output stream.
    @param out is the output stream.
    @param card is the card to be printed. */
std::ostream& operator<< 
	(std::ostream &out, const TMCG_Card &card);

#endif
