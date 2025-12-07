#pragma once

enum TokenType : int {
	TId = 1,
	TInt = 2,
	TDouble = 3,
	TClass = 4,
	TReturn = 5,
	TMain = 6,
	TWhile = 7,
	TVoid = 8,

	// константы
	TConstInt = 10,
	TConstDouble = 11,

	// пунктуация и операторы
	TPoint = 20,
	TComma = 21,
	TSemicolon = 22,
	TLB = 23,   // (
	TRB = 24,   // )
	TLFB = 25,  // {
	TRFB = 26,  // }

	// сравнения и логика
	TL = 30,    // <
	TG = 31,    // >
	TLE = 32,   // <=
	TGE = 33,   // >=
	TNotEq = 34,// !=
	TEq = 35,   // ==
	TEval = 36, // =

	// арифметика
	TMinus = 37,
	TPlus = 38,
	TMult = 39,
	TDiv = 40,
	TMod = 41,

	TMinusEq = 42, // -=
	TPlusEq = 43,  // +=
	TMultEq = 44,  // *=
	TDivEq = 45,   // /=
	TModEq = 46,   // %=

	TDec = 47, // --
	TInc = 48, // ++

	// служебные
	TEnd = 100,
	Terr = 200
};

