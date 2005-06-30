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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
*******************************************************************************/

#ifndef INCLUDED_VTMF_Card_HH
	#define INCLUDED_VTMF_Card_HH

	// config.h
	#if HAVE_CONFIG_H
		#include "config.h"
	#endif

	// C and STL header
	#include <cassert>
	#include <string>
	#include <iostream>
	
	// GNU multiple precision library
	#include <gmp.h>
	
	#include "parse_helper.hh"

/** @brief A data structure for cards.
    
    This struct represents a card in the discrete logarithm instantiation
    of the general cryptographic primitive "Verifiable-l-out-of-l Threshold
    Masking Function" by Barnett and Smart [BS03]. */
struct VTMF_Card
{
	/** @f$c_1, c_2\in G@f$ encode the type of the card. They should always
	    be elements of the finite abelian group @f$G@f$ in which the DDH problem
	    is believed to be hard. */
	mpz_t c_1, c_2;
	
	/** This constructor initalizes all necessary resources. */
	VTMF_Card
		();
	
	/** A simple copy-constructor.
	    @param that is the card to be copied. */
	VTMF_Card
		(const VTMF_Card& that);
	
	/** A simple assignment-operator.
	    @param that is the card to be assigned. */
	VTMF_Card& operator =
		(const VTMF_Card& that);
	
	/** This operator tests two cards for equality. */
	bool operator ==
		(const VTMF_Card& that) const;
	
	/** This operator tests two cards for inequality. */
	bool operator !=
		(const VTMF_Card& that) const;
	
	/** This function imports the card.
	    @param s is correctly formated input string.
	    @returns True, if the import was successful. */
	bool import
		(std::string s);
	
	/** This destructor releases all occupied resources. */
	~VTMF_Card();
};

/** @relates VTMF_Card
    This operator prints a card to an output stream.
    @param out is the output stream.
    @param card is the card to be printed. */
std::ostream& operator<< 
	(std::ostream &out, const VTMF_Card &card);

#endif
