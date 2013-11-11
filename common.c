/* Common global definitions */

#include "nml.h"



const int NML_NN_DOMAINS [] = {AF_SP, AF_SP_RAW};
const char NML_DOMAIN_STRINGS [][16] = {
	"sp_socket", "raw_sp_socket"
};



const int NML_NN_PROTOCOLS[] =  {NN_PAIR, NN_PUB, NN_SUB, NN_REQ,
	NN_REP, NN_PUSH, NN_PULL, NN_SURVEYOR, NN_RESPONDENT, NN_BUS};

const char NML_PROTOCOL_STRINGS [][16] = 
 { "pair", "pub", "sub" , "req", "rep", "push", "pull", "surveyor", "respondent", "bus"};

const int NML_NN_DOMAINS_COUNT = sizeof(NML_NN_DOMAINS) / sizeof(* NML_NN_DOMAINS);
const int NML_PROTOCOL_COUNT = sizeof(NML_NN_PROTOCOLS) / sizeof( * NML_NN_PROTOCOLS);