/* 
	CIVcmdsRotor.h - Library for a rotator control communication via ICOM's CI-V bus
	Created by Wilfried Dilling, DK8RW, May 30, 2021
	Released into the public domain
	
	Definition of ICOM command set (only sub set as required!)
	To be included into CIV.cpp and all modules which may use one or more of 
	these definitions
*/
#ifndef CIVcmdsRotor_h
#define CIVcmdsRotor_h

//------------------------------------------------------------------------
//
// Interface definition of the homebrew Rotator Control(according Yaesu GS-232A)
//
//
// -> "C"       get the current antenna direction
// <- "+0nnn"   returnvalue is a 2 byte BCD coded direction in 0 .. 360 degrees
// -> "L"   		rotate the antenna to the left (CCW)
// -> "R"   		rotate the antenna to the left (CCW)
//
// -> "A"       stop rotation
// -> "S"       stop everything
//
// -> "Mnnn"  	set antenna direction directly
//
//  where : nnn is in Yaesu-degrees, i.e. 0 .. 360 degrees from the left final 
//  position (CCW) to the right final position (CW)
//  if rotator is active, ROTOR_ACTIVE_OFFSET is added to nnn
//  ("real degrees" are the degrees on a world map (-180 .. +180 deg, 
//   in the case of end position south) 
//------------------------------------------------------------------------


// command "body" of the CIV commands for the homebrew rotor-device

constexpr uint8_t CIV_C_R_C[] = {1,0x70}; // get current antenna direction 
																					// (return: 0nnn where nnn is the 
																					// direction in 2 bytes BCD)

#define ROTOR_ACTIVE_OFFSET 500						// if rotator is currently active, this offset
																					// will be added to the direction 

constexpr uint8_t CIV_C_R_A[] =	{1,0x71}; // stop rotation
constexpr uint8_t CIV_C_R_S[] =	{1,0x72}; // stop everything

constexpr uint8_t CIV_C_R_L[] =	{1,0x73}; // rotate left  (CCW)
constexpr uint8_t CIV_C_R_R[] =	{1,0x74}; // rotate right (CW)

constexpr uint8_t CIV_C_R_M[] = {1,0x75}; // set wanted antenna direction 
																					// (Mnnnn, i.e. 2 data bytes with 
																					// the BCD coded direction)



#endif
