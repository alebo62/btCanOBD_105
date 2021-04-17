#include "messages.h"
//static unsigned char c;
//void checksum (unsigned char *ip)
//{
//  int sum = 0; /* assume 32 bit long, 16 bit short */
//  unsigned char* ips = ip;
//  int len = ip[9];
//  ip[10] = ip[11] = 0;

//  while ( len >1 )
//  {
//    sum += (*ip);
//    ip++;
//    sum += ((unsigned short ) *ip) << 8;
//    ip++;
//    if (sum >>16) /* if high-order bit set, fold */
//      sum = (sum & 0xFFFF) + (sum>> 16) ;
//    len -= 2;
//  }

//  if ( len ) /* take care of left over byte */
//  {
//    unsigned char end[2];
//    end[0] = * (unsigned char *) ip;
//    end[1] = 0;
//    sum += * (unsigned short *) end;
//  }
//  while ( sum >> 16)
//    sum =(sum & 0xFFFF) + (sum>> 16);

//  sum = ~sum;
//  ips[10] = sum & 0xFF;
//  ips[11] = (sum >> 8) & 0xFF;

//}

////Checksum = ~ ( Opcode + # of bytes + Payload ) + 0x33
//void checksum_pep_no_pld(hrnp_no_pld_t *ip)
//{
//  ip->pep_npld.Checksum = (~(ip->pep_npld.opcode[0] + ip->pep_npld.opcode[1] + ip->pep_npld.num_of_bytes[0] + \
//      ip->pep_npld.num_of_bytes[1])) + 0x33;
//}

//void checksum_pep(hrnp_t *ip)
//{
//  unsigned char sum = 0;
//  unsigned char cnt = 0;
//  unsigned char i = ip->pep.num_of_bytes[0];
//  sum = ip->pep.opcode[0] + ip->pep.opcode[1] + ip->pep.num_of_bytes[0] + ip->pep.num_of_bytes[1];
//  while(cnt < i)
//  {
//    sum += ip->pep.pld[cnt++];
//  }
//  ip->pep.Checksum = ~sum + 0X33;
//}


void checksum (unsigned char *ip)
{
  unsigned char sum_pep = 0;
  unsigned char cnt_pep = 0;
  unsigned char i_pep;// = ip->pep.num_of_bytes[0];
  
	
  int sum = 0; /* assume 32 bit long, 16 bit short */
  unsigned char* ips = ip;
  int len = ip[9];
	
  ip[10] = ip[11] = 0;

  if(len == 19)
  {
    ip[17] = ~(ip[13] + ip[14] + ip[15] + ip[16]) + 0x33;
  }
  else if(len > 19)
  {
		if(ip[12] == 2) // RCP little endian
			i_pep = ip[15];
		else  					// LP TMP big endian
			i_pep = ip[16];
		
    sum_pep = ip[13] + ip[14] + ip[15] + ip[16];
    while(cnt_pep < i_pep)
      sum_pep += ip[17 + (cnt_pep++)];
    ip[len - 2] = ~sum_pep + 0x33;
  }

  while( len > 1 )
  {
    sum += (*(unsigned short*)ip);
    ip +=2;
    if (sum >>16) /* if high-order bit set, fold */
      sum = (sum & 0xFFFF) + (sum>> 16) ;
    len -= 2;
  }

  if ( len ) /* take care of left over byte */
  {
    unsigned char end[2];
    end[0] = * (unsigned char *) ip;
    end[1] = 0;
    sum += * (unsigned short *) end;
  }
  while ( sum >> 16)
    sum =(sum & 0xFFFF) + (sum>> 16);

  sum = ~sum;
  ips[10] = sum & 0xFF;
  ips[11] = (sum >> 8) & 0xFF;
}
