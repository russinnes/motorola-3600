#include "stdafx.h"
#include "motorola.h"

static int mapInpFreq(int)
{
  return 0;
}

inline bool isHybrid(char bankcode)
{
  return (bankcode >= 'a' && bankcode <= 'n'); 
}

inline bool isTypeI(char bankcode)
{
  return (bankcode >= 'A' && bankcode <= 'N'); 
}

inline int typeToIdx(char bankcode)
{
  return isHybrid(bankcode) ? (bankcode - 'a') : (bankcode - 'A');
}

inline bool isTypeIorHybrid(char bankcode)
{
  return ( isHybrid(bankcode) || isTypeI(bankcode)); 
}

inline bool isTypeII(char bankcode)
{
  return (bankcode == '2');
}

inline unsigned short typeIgroup(unsigned short g, char bankcode)
{
  return (unsigned short)(g & ~idMask[typeToIdx(bankcode)]);
}

inline unsigned short typeIradio(unsigned short g, char bankcode)
{
  return (unsigned short)(g & radioMask[typeToIdx(bankcode)]); // omit subfleet
}

inline char makeHybrid(char bankcode)
{
  return (char)((bankcode - 'A') + 'a');
}

static int typeIgroup(int nGroup, int nBank)
{
  return nGroup;
}

static int typeIradio(int nRadio, int nBank)
{
  return nRadio;
}

static void type2Gcall(int nFreq, int nGroup, int nRadio, int nFlags = 0)
{
  printf("%d F %d G %d R\n", nFreq, nGroup, nRadio);
}

static void typeIGcall(int nFreq, int nGroup, int nRadio, int nFlags = 0)
{
  printf("%d F %d G %d R\n", nFreq, nGroup, nRadio);
}

static void hybridCall(int nFreq, int nGroup, int nRadio, int nFlags = 0)
{
  printf("%d F %d G %d R\n", nFreq, nGroup, nRadio);
}

static void hybridGcall(int nFreq, int nGroup, int nRadio, int nFlags, int nOther = 0)
{
  printf("%d F %d G %d R\n", nFreq, nGroup, nRadio);
}

static void type0Gcall(int nFreq, int nGroup, int nRadio, int nFlags = 0)
{
  printf("%d F %d G %d R\n", nFreq, nGroup, nRadio);
}

static void type0Icall(int nFreq, int nGroup, int nRadio, bool, bool, bool)
{
  printf("%d F %d G %d R\n", nFreq, nGroup, nRadio);
}

/*
This function is called with the following on the stack:

[2] <calling radio-type 2>   <G/I>   <don't care>
[1] <destination ID or GRP>  <G/I>   <frequency #>
        
it is used for: 

    type II calls (initial)
	type II radio masquerading as Type 1
	Astro call
	Coded PC grant
	Hybrid call of type II radio in type 1 bank
*/
void CMotorolaSystem::twoOSWcall(unsigned short flags)
{
  unsigned short blockNum;
  char banktype;

  if( stack[1].grp )
  {
    blockNum = (unsigned short)((stack[1].id>>13)&7);
    //banktype = types[blockNum];
    banktype = ' ';
    if( banktype == '?' )
	{
      //types[blockNum] = '2';
      //note_type_2_block(blockNum);
      banktype = '2';
    }
    if( isTypeI(banktype) )
    {
       /* types[blockNum] = */ banktype = makeHybrid(banktype);
    }
    if( isTypeII(banktype) )
    {
      type2Gcall(stack[1].cmd,								// frequency 
        (unsigned short)(stack[1].id & 0xfff0),				// group ID
        stack[2].id,										// calling ID
        (unsigned short)((stack[1].id & 0x000f) | flags) ); // flags
     }
     else if( isTypeI(banktype) )
     {
       typeIGcall(stack[1].cmd,                             // frequency
                  banktype,
                  typeIgroup(stack[1].id,banktype),			// group ID
                  typeIradio(stack[1].id,banktype) );		// calling ID
	 }
	 else if ( isHybrid(banktype) )
     {
       hybridGcall(stack[1].cmd,                            // frequency
                                banktype,
                                typeIgroup(stack[1].id,banktype),       // group ID
                                stack[2].id,                            // true ID
                                typeIradio(stack[1].id,banktype)        // alias ID
                        );
     }
	 else
	 {
       type0Gcall(stack[1].cmd,  // frequency 
         stack[1].id,            // group ID
         stack[2].id,            // calling ID
         flags);                 // flags - don't get from group low bits
     }
  }
  else
  {
    type0Icall(stack[1].cmd,     // frequency
               stack[1].id,      // destination
               stack[2].id,      // talker
               false, false,(flags & OTHER_DIGITAL)?true:false );
  }
}

void CMotorolaSystem::show_good_osw (struct OSW &osw)
{
  // do something with the OSW
  unsigned short blockNum;
  char banktype;
  unsigned short tt1,tt2;

  if( osw.cmd == OSW_BACKGROUND_IDLE )
  {
    // idle word - normally ignore but may indicate transition into
    // un-numbered system or out of numbered system
    if( ++idlesWithNoId > 20 && (osw_state == eIdentifying || osw_state == eOperNewer) )
    {
      //noteOlderType();
      osw_state = eGettingOldId;
    }
    // returning here because idles sometimes break up a 3-OSW sequence!
    return;
  }
        
  if( osw.cmd >= OSW_AMSS_ID_MIN && osw.cmd <= OSW_AMSS_ID_MAX )
  {
    // on one system, the cell id also got stuck in the middle of the 3-word sequence!
    // Site ID
    if(m_bIsNetworkable)
    {
      m_nNoteSite = osw.cmd - OSW_AMSS_ID_MIN;
    }
    else
    {
      if( ++netCounts > 5 )
      {
        m_bIsNetworkable = true;
        netCounts = 0;
        }
      }
      idlesWithNoId = 0;
      // returning here because site IDs sometimes break up a 3-OSW sequence!
      return;
    }
    
    // maintain a sliding stack of 5 OSWs. If previous 
    // iteration used more than one, don't utilize stack 
    // until all used ones have slid past.
    switch(numStacked) // note: drop-thru is intentional!
    {
        case 5:
        case 4:
          stack[4] = stack[3];
        case 3:
          stack[3] = stack[2];
        case 2:
          stack[2] = stack[1];
        case 1:
          stack[1] = stack[0];
        case 0:
          stack[0] = osw;
          break;
        default:
          //shouldNever("corrupt value for nstacked");
          break;
        }
        if(numStacked < 5) ++numStacked;

        if( numConsumed > 0)
        {
          if(--numConsumed > 0) return;
        }
		// at least need a window of 3 and 5 is better.
        if(numStacked < 3) return; 

        // look for some larger patterns first... 
		// parts of the sequences could be
        // mis-interpreted if taken out of context.
    
        if (stack[2].cmd == OSW_FIRST_NORMAL || stack[2].cmd == OSW_FIRST_TY2AS1)
        {
          switch(stack[1].cmd)
          {
            case OSW_EXTENDED_FCN:
        
              if(stack[1].id & 0xfff0 == 0x26f0)
			  {
				// ack type 2 text msg
                if(m_bVerbose)
				{
                  //note_text_msg(stack[2].id,"Msg",stack[1].id & ((unsigned short)15));
                }
              }
			  else if(stack[1].id & 0xfff8 == 0x26E0)
			  { // ack type 2 status
                if(m_bVerbose)
				{
                  //note_text_msg(stack[2].id,"Status",stack[1].id & ((unsigned short)7));
                }
              }
			  else if( stack[0].isFreq && (stack[0].id & 0xff00) == 0x1F00 &&
                (stack[1].id & 0xfc00) == 0x2800 && stack[0].cmd == (stack[1].id & 0x3FF))
              {
                /* non-smartzone identity:  sysid[10]
                   <sysid>              x   308
                   <001010><ffffffffff> x   30b
                   1Fxx                 x   <ffffffffff>
                */ 
                //noteIdentifier(stack[2].id,false);
                //noteDataChan(stack[0].cmd);
                numConsumed = 3; // we used up all 3
                return;
              }
			  else if((stack[1].id & 0xfc00) == 0x2800)
              {
                /* smartzone identity: sysid[10]

                  <sysid>              x   308
                  <001010><ffffffffff> x   30b
                */ 
                //noteIdentifier(stack[2].id,false);
                //noteDataChan((unsigned short)(stack[1].id & 0x03ff));
              }
			  else if((stack[1].id & 0xfc00) == 0x6000)
              {
                /* smartzone peer info:                         sysid[24]
                  <sysid>              x   308
                  <011000><ffffffffff> x   30b
                  */ 
                  // evidence of smartzone, but can't use this pair for id
                  idlesWithNoId = 0; // got identifying info...
                  if( m_bIsNetworkable && !m_bIsNetworked)
				  {
                    m_bIsNetworked = true;
                  }
			  }
			  else if((stack[1].id & 0xfc00) == 0x2400)
              { // various
                idlesWithNoId = 0;
                if( (stack[1].id & 0x03ff) == 0x21c)
				{
                  if(m_bVerbose)
                  {
                    //note_affiliation(stack[2].id,0);
                  }
                }
              }
			  else if( stack[1].id  == 0x2021)
			  {
                  //mscancel(stack[2].id);
              }
			  else
			  {
				idlesWithNoId = 0;

         /* unknown sequence.
         example: <000000> fffa x 308 then 0321 x 30b gateway close?
         example: <001000> 3890 G 308      2021 G 30b 
                  <001011> 811d I 308      2c4a I 30b   
                  <001001> 83d3 I 308      261c I 30b
                                                              
         known:   <011000>
                  <001010>   
                                                        
                                                        
          */                                           
			  }
              numConsumed = 2;
              return; 

			case 0x320:
              if(stack[0].cmd == OSW_EXTENDED_FCN)
              {
                // definitely smart-zone: 308 320 30b
                //  <sysId>                             308
                //  <cell id 6 bits><10 bits see below> 320
                //      <011000><ffffffffff>            30b
                //
                /*
                  10 bits:  bbb0v?AaES

                  bbb - band: [800,?,800,821,900,?,?]
                  32  v   - VOC
                  8   A   - Astro
                  4   a   - analog
                  2   E   - 12kbit encryption
                  1   S   - site is Active (if zero?)
                */
                numConsumed = 3;
                idlesWithNoId = 0; // got identifying info...
                if( (stack[0].id & 0xfc00) == 0x6000)
                {
                  if( (stack[2].id == m_nSysId) )
                  {
                    //note_neighbor((unsigned short)((stack[1].id>>10) & 0x3F),(unsigned short)(stack[0].id & 0x3ff));
                    if( m_bIsNetworkable && !m_bIsNetworked)
                    {
                      m_bIsNetworked = true;
                    }
                    // if this describes 'us', note any astro or VOC characteristics
                    // for alternate data channel, note that.
					if( m_bIsNetworked && ( ( (stack[1].id>>10) & 0x3F) ==  m_nSiteId) )
					{
                      if(stack[0].grp)
					  {
						// if active data channel
                        if( stack[1].id & 8 )
						{
                          //setSubType(" Astro");
                        }
                        if( stack[1].id & 32 )
						{
                          //setOneChannel("/VOC");
                        }
                      }
					  else
					  {
						// is an alternate data channel frequency
                        //noteAltDchan((unsigned short)(stack[0].id & 0x3ff));
                      } 
                    }
                  }
				}
                return;         
              }
              break;
                        
        case OSW_TY2_AFFILIATION:
            // type II requires double word 308/310. single word version is type 1 status,
            // whatever that means. 
            if(m_bVerbose)
            {
                    //note_affiliation(stack[2].id,stack[1].id);

            }
            break;

        case OSW_TY2_MESSAGE:
			// sprintf(tempArea,"Radio %04x, Message %d [1..8]\n",stack[2].id,1+(stack[1].id&7));
            if(m_bVerbose)
            {
                    //note_text_msg(   stack[2].id,
                    //                        "Msg",
                    //                        (unsigned short)(stack[1].id&7)
                    //                    );    
            }
            break;
                                        
        case OSW_TY2_CALL_ALERT:
			// type II call alert. we can ignore the 'ack', 0x31A
			if(m_bVerbose)
			{
					//note_page(stack[2].id, stack[1].id);
			}
			break;
        
        case OSW_SYSTEM_CLOCK:  // system clock
			tt1 = stack[2].id;
			tt2 = stack[1].id;
			if( tt2 != ott2 || tt1 != ott1 ) 
			{
				char tempArea[64];
				ott2 = tt2;
				ott1 = tt1;
				sprintf(tempArea,"%02u/%02u/%02u %02u:%02u",
				(tt1>>5)& 0x000f,
				(tt1)   & 0x001f,
				(tt1>>9),
				(tt2>>8)& 0x001f,
				(tt2)   & 0x00ff);
				//QuickOut(STATROW+1,66,color_norm,tempArea);
			}
            break;
                        
        case OSW_EMERG_ANNC:    
            //noteEmergencySignal(stack[2].id);        
            break;  
        case OSW_AFFIL_FCN:
            switch(stack[1].id & 0x000f)
            {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
                    if(m_bVerbose)
                    {
                            //note_text_msg(   stack[2].id,
                            //                        "Status",
                            //                        (unsigned short)(stack[1].id & 0x0007)
                            //                );
                    }
                    break;
				case 8:
                    //noteEmergencySignal(stack[2].id);
                    break;
				case 9: // ChangeMe request
				case 10:// Ack Dynamic Group ID
				case 11:// Ack Dynamic Multi ID
                    break;
				default:
                    break;
            }      
            break;  
        
        // Type I Dynamic Regrouping ("patch")
        
        case 0x340: // size code A (motorola codes)
        case 0x341: // size code B
        case 0x342: // size code C
        case 0x343: // size code D
        case 0x344: // size code E
        case 0x345: // size code F
        case 0x346: // size code G
        case 0x347: // size code H
        case 0x348: // size code I
        case 0x349: // size code J
        case 0x34a: // size code K
            // patch notification
            // patch 'tag' is in stack[1].id
            //group associated is stack[2].id
                    
            switch(stack[2].id & 7)
            {
                case 0: // normal for type 1 groups ???
                case 3: // patch
                case 4: // emergency patch
                    //note_patch(/*tag*/ stack[1].id, /*group*/ stack[2].id,
                    //        (char)('A'+(stack[1].cmd - 0x340)) );
                    break;

                case 5:
                case 7:
                    //note_patch(/*tag*/ stack[1].id, /*group*/ stack[2].id,
                    //        (char)('A'+(stack[1].cmd - 0x340)) ,true);
                    break;

                default:
                    break;
            }
            nowFine -= 2; // time machine - these get in the way of notseen calc 
            break;

        case 0x34c: // size code M trunker = L
            // patch notification
            // patch 'tag' is in stack[1].id
            //group associated is stack[2].id
            switch(stack[2].id & 7)
            {
                case 0: // normal for type 1 groups ???
                case 3: // patch
                case 4: // emergency patch
                
                    
                    //note_patch(/*tag*/ stack[1].id, /*group*/ stack[2].id,'L');
                    break;
                case 5:
                case 7:
                    //note_patch(/*tag*/ stack[1].id, /*group*/ stack[2].id,
                    //        'L', true);
                    break;
                default:
                    break;
            }
            nowFine -= 2; // time machine - these get in the way of notseen calc 
            break;
        case 0x34e: // size code O Trunker = M
            // patch notification
            // patch 'tag' is in stack[1].id
            //group associated is stack[2].id
            switch(stack[2].id & 7)
            {
                case 0: // normal for type 1 groups ???
                case 3: // patch
                case 4: // emergency patch
                
                    
            //note_patch(/*tag*/ stack[1].id, /*group*/ stack[2].id,'M');
                    break;
                case 5:
                case 7:
                    //note_patch(/*tag*/ stack[1].id, /*group*/ stack[2].id, 'M',true);
                    break;
                default:
                    break;
            }
            nowFine -= 2; // time machine - these get in the way of notseen calc 
            break;  
        
        case 0x350: // size code Q Trunker = N
        
            // patch notification
            // patch 'tag' is in stack[1].id
            //group associated is stack[2].id
            switch(stack[2].id & 7)
            {
                case 0: // normal for type 1 groups ???
                case 3: // patch
                case 4: // emergency patch
                
					//note_patch(/*tag*/ stack[1].id, /*group*/ stack[2].id,'N');
                    break;
                case 5:
                case 7:
                    //note_patch(/*tag*/ stack[1].id, /*group*/ stack[2].id, 'N', true);
                    break;
                default:
                    break;
            }
            nowFine -= 2; // time machine - these get in the way of notseen calc 
            break;                  
        default:

            if(stack[1].isFreq)
            {
                if ( stack[2].cmd == OSW_FIRST_TY2AS1 && stack[1].grp)
                {
                    blockNum = (unsigned short)((stack[1].id>>13)&7);
					banktype = ' ';
                    //banktype = types[blockNum];
                    if( banktype == '2' )
                    {
						char tempArea[64];
                        //types[blockNum] = '?';
                        //_settextposition(FBROW,1);
                        sprintf(tempArea,"Block %d <0..7> is Type IIi                         ",blockNum);
                        //_outtext(tempArea);
                    }
                    //setSubType("i");
                }
                if( stack[2].id != 0 )
                {
			      // zero id used special way in hybrid system
                  twoOSWcall();
                }
            }
            // otherwise 'busy', others...
        }
        numConsumed = 2;
        return;
        }

        // uhf/vhf equivalent of 308/320/30b

        if( stack[1].cmd == 0x0320 && stack[0].cmd == OSW_EXTENDED_FCN && stack[2].isFreq)
        {
          numConsumed = 3;
          idlesWithNoId = 0; // got identifying info...
          if( (stack[0].id & 0xfc00) == 0x6000)
          {
            if(stack[2].id == m_nSysId)
            {
              //if( myFreqMap->isFrequency((unsigned short)(stack[0].id&0x3ff),getBandPlan()) )
              //{
              //  note_neighbor((unsigned short)((stack[1].id>>10) & 0x3F),(unsigned short)(stack[0].id & 0x3ff));
              //}
              if( m_bIsNetworkable && !m_bIsNetworked)
              {
		        m_bIsNetworked = true;
              }

              // if this describes 'us', note any astro or VOC characteristics
              // if this describes an alternate data frequency, note that.
                                
              if(m_bIsNetworked && ((unsigned short)((stack[1].id>>10) & 0x3F) ==  m_nSiteId) )
			  {
                if(stack[0].grp)
				{ // if active data channel
                  if( stack[1].id & 8 )
				  {
                    //setSubType(" Astro");
                  }
                  if( stack[1].id & 32 )
				  {
                    //setOneChannel("/VOC");
                  }
                }
				else
				{  // is an alternate data channel frequency
                  //noteAltDchan((unsigned short)(stack[0].id & 0x3ff));
                } 
              }
            }
          }
          return;
        }
        /* astro call or 'coded pc grant' */
        if( (stack[2].cmd == OSW_FIRST_ASTRO || stack[2].cmd == OSW_FIRST_CODED_PC) && stack[1].isFreq)
        {
          if( stack[2].id )
          {
            if(stack[2].cmd == OSW_FIRST_ASTRO)
			{
              //setSubType(" Astro");
            }
            twoOSWcall(OTHER_DIGITAL);
          }
          numConsumed = 2;
          return;
        }   
   
    // have handled all known dual-osw sequences above. these tend to be single or correlated
    
        switch( stack[2].cmd )
        {
          case OSW_SYS_NETSTAT:
            if(!m_bIsNetworkable)
            {
              if( ++netCounts > 5 )
              {
                m_bIsNetworkable = true;
                netCounts = 0;
              }
            }
            idlesWithNoId = 0;
            break;
                
          case OSW_SYS_STATUS: // system status
			{
              register unsigned short statnum;

              statnum = (unsigned short)((stack[2].id >> 13) & 7); // high order 3 bits opcode
              if(statnum == 1)
			  {
                if(stack[2].id & ((unsigned short)0x1000))
				{
                  //setBasicType("Type II");             
                }
				else
				{
                  //setBasicType("Type I");
                } 
                statnum = (unsigned short)(stack[2].id >> 5);
                statnum &= 7;
                //setConTone(tonenames[statnum]);
              }
            }          
            break;
                        
        case OSW_SCAN_MARKER:
            //noteIdentifier(stack[2].id,false);
            if( osw_state == eOperNewer )
            {
              //setBasicType("Type II");
            }
            break;
                
        case OSW_TY1_EMERGENCY:         // type 1 emergency signal       
            if(numStacked > 3 )
            { // need to be unambiguous with type 2 call alert 
              blockNum = (unsigned short)((stack[2].id>>13)&7);
              //banktype = types[blockNum];
			  banktype = ' ';
              if ( isTypeI(banktype) || isHybrid(banktype))
			  {
                //noteEmergencySignal(typeIradio(stack[2].id,banktype)); 
              }
			  else
			  {
                //noteEmergencySignal(stack[2].id); 
              }
            }
            break;
                
        case OSW_TY1_ALERT:
            blockNum = (unsigned short)((stack[2].id>>13)&7);
            //banktype = types[blockNum];
			banktype = ' ';
            if( isTypeI(banktype) || isHybrid(banktype)) {
                //note_page(typeIradio(stack[2].id,banktype));
            } else {
                //note_page(stack[2].id);
            }
            break;
                      
        case OSW_CW_ID: // this seems to catch the appropriate diagnostic. thanks, wayne! 
			if( (stack[2].id & 0xe000)  == 0xe000)
			{
			  int cwFreq = (unsigned short)(stack[2].id & 0x3ff);
			  //noteCwId(cwFreq, stack[2].id & 0x1000);
			} else
			{
              static unsigned int lastDiag = 0;
              static time_t lastT = 0;
              if( (stack[2].id != lastDiag) || ((now - lastT) > 120))
              {
                switch(stack[2].id & 0x0F00)
				{
                  case 0x0A00:
                    //sprintf(tempArea,"Diag %04x: %s Enabled",stack[2].id,getEquipName(stack[2].id&0x00ff));
                    break;
                  case 0x0B00:
                    //sprintf(tempArea,"Diag %04x: %s Disabled",stack[2].id,getEquipName(stack[2].id&0x00ff));
                    break;
                  case 0x0C00:
                    //sprintf(tempArea,"Diag %04x: %s Malfunction",stack[2].id,getEquipName(stack[2].id&0x00ff));
                    break;
                  default: 
                    break;
                    //sprintf(tempArea,"Diagnostic code (other): %04x",stack[2].id);
                }
                //logStringT(tempArea,DIAGNOST);
                lastT = now;
                lastDiag = stack[2].id;
              }
            }
            break;

        default:
            if(stack[2].cmd >= OSW_TY1_STATUS_MIN &&
               stack[2].cmd <= OSW_TY1_STATUS_MAX &&
               numStacked > 4) {

                // place to put type 1 status messages XXX
                if(m_bVerbose)
				{
                    //note_text_msg(   stack[2].id, "Status", (unsigned short)(stack[2].cmd - OSW_TY1_STATUS_MIN) );
                }
                break;
            }
                    
            // this is the most frequent case that actually touches things....
                    
            if (stack[2].isFreq && (numStacked > 4)  )
			{
              if( m_bVhfUhf &&           // we have such a system
                 stack[1].isFreq &&      // this is a pair of frequencies
                 (mapInpFreq(stack[2].cmd)==stack[1].cmd)  // the first frequency is input to second
                  )
			  {
                /*
                   special processing for vhf/uhf systems. They use 
				   the following pairs during call setup:
                   <originating radio id>           G/I <input frequency>
                   <destination group or radio> G/I <output frequency>
                */
                if( stack[1].grp )
				{
                  blockNum = (unsigned short)((stack[1].id>>13)&7);
                  //banktype = types[blockNum];
                  // on uhf/vhf systems, only allowed to be type II!
				  banktype = ' ';
                  if ( banktype != '0' && banktype != '2' )
				  {
                    //types[blockNum] = '2';
                    //note_type_2_block(blockNum);
                  }
                }
                if( stack[1].grp && !stack[2].grp )
				{
				  // supposedly astro indication
                  twoOSWcall(OTHER_DIGITAL);
                }
				else
				{
                  twoOSWcall();
                }
                numConsumed = 2;
                return; // the return below pops only one word, we used 2 here
                            
              }
			  else
			  {
                // bare assignment 
                // this is still a little iffy...one is not allowed to use IDs that
                // mimic the system id in an old system, so seems to work ok...
                if(((stack[2].id & 0xff00) == 0x1f00) && (osw_state == eOperOlder))
				{
                  //noteIdentifier(stack[2].id,true);
                  //noteDataChan(stack[2].cmd);
                }
				else
				{
                            
                  // in the case of late entry message, just tickle current one
                  // if possible.... preserve flags since may be lost in single
                  // word case
                                
                  if( stack[2].grp)
				  {
                    blockNum = (unsigned short)((stack[2].id>>13)&7);
                    //banktype = types[blockNum];
					banktype = ' ';
                    if( isTypeII(banktype) )
					{
                      type2Gcall(stack[2].cmd,                    // frequency 
                        (unsigned short)(stack[2].id & 0xfff0),   // group ID
                        (unsigned short)(stack[2].id & 0x000f) ); // flags
                    }
					else if(isTypeI(banktype))
					{
                      typeIGcall(stack[2].cmd, banktype, 
						  typeIgroup(stack[2].id,banktype),       // group ID
                          typeIradio(stack[2].id,banktype) );     // calling ID
                    }
					else if(isHybrid(banktype))
					{
                      hybridGcall(stack[2].cmd,                 // frequency
                        banktype,
                        typeIgroup(stack[2].id,banktype),       // group ID
                        typeIradio(stack[2].id,banktype) );     // calling ID
                    }
					else
					{
                      type0Gcall(stack[2].cmd,	// frequency
                        stack[2].id,            // group ID
                        0);                     // no flags
                    }
                  }
				  else
				  {
                    type0Icall(stack[2].cmd,  // freq#
                      stack[2].id,       // talker
					  0, false,false,false);
                  }
                }
              }
            }
			else if( osw_state == eGettingOldId )
			{
              if (stack[2].isFreq && ((stack[2].id & 0xff00) == 0x1f00))
			  { 
                //noteIdentifier(stack[2].id,true);
                //noteIdentifier(stack[2].id,true);
              }
            }
            break;
        }
        numConsumed = 1;
        return;
}
//
// process de-interleaved incoming data stream 
// inputs: 0 zero bit
//         1 one bit
// once an OSW is received it will be placed into the buffer 
// m_bosw[] where m_bosw[0] is the most recent entry.
//
void CMotorolaSystem::process_osw(bool sl)
{
  register int l;
  int sr, sax, f1, f2, iid, cmd, neb;

  // accumulate bits into the buffer.
  m_gob[m_ct] = sl;
  m_ct++;

  if (m_ct == 76)
  {
    sr = 0x036E;
    sax = 0x0393;
    neb = 0;

    //
    // run through convolutional error correction routine 
    // Reference : US PATENT # 4055832                        
    //
    for (l = 0; l < 76; l += 2)
    {
      // osw gets the DATA bits
      m_osw [l >> 1] = m_gob [l];

	  // m_gob becomes the syndrome
      if (m_gob [l])
      {
        m_gob [l]     ^= 0x01;
        m_gob [l + 1] ^= 0x01;
        m_gob [l + 3] ^= 0x01;
      }
    }

    //
    //  Now correct errors
    //
    for (l = 0; l < 76; l += 2)     
    {
      if ((m_gob [l + 1]) && (m_gob [l + 3]))
      {
        m_osw[l >> 1] ^= 0x01;
        m_gob [l + 1]  ^= 0x01;
        m_gob [l + 3]  ^= 0x01;
      }
    }

    //
    //  Run through error detection routine 
    //
    for (l = 0; l < 27; l++)
    {
      if (sr & 0x01) 
        sr = (sr >> 1) ^ 0x0225; 
      else 
        sr = sr >> 1;

      if (m_osw[l]) 
        sax = sax ^ sr;
	}

    for (l = 0; l < 10; l++)
    {
      f1 = (m_osw[36 - l]) ? 0 : 1;
      f2 = sax & 0x01;

      sax = sax >> 1;

      //
      // neb counts # of wrong bits
      //
      if (f1 != f2) 
        neb++;
    }
    //
    // if no errors - OSW received properly; process it 
    //
    if (neb == 0)
    {
      for (iid = 0, l = 0; l < 16; l++)
      {
        iid = iid << 1;

        if (!m_osw[l])  iid++;
      }

      m_bosw.id = (unsigned short)(iid ^ 0x33c7);
      m_bosw.grp = (m_osw[16] ^ 0x01) != 0;

      for (cmd = 0, l = 17; l < 27; l++)
      {
        cmd = cmd << 1;

        if (!m_osw[l]) 
          cmd++;
      }

      m_bosw.cmd = (unsigned short)(cmd ^ 0x032a);
      ++m_nGoodCount;

	  // TODO: make something of this ...
      //if( mySys )
	  //{
      //  m_bosw.isFreq = myFreqMap->isFrequency(m_bosw.cmd,getBandPlan());
      //}
	  //else
	  //{
      // m_bosw.isFreq = myFreqMap->isFrequency(m_bosw.cmd,'-');
      //}
      show_good_osw(m_bosw);
	}
    else
	{
	  ++m_nBadCount;
	}
  }
}
//
// de-interleave OSW stored in array m_ob[] and process 
//
void CMotorolaSystem::pigout(int nSkipOver)   
{
  // reset the m_gob counter.
  m_ct = 0;

  for (int i1 = 0; i1 < 19; ++i1)
  {
    for (int i2 = 0; i2 < 4*19; i2 += 19)
	{
      process_osw( m_ob[i2 + i1 + nSkipOver] );
    }
  }
}
//
// this routine looks for the frame sync and splits off the
// 76 bit data frame and sends it on for further processing.
//
void CMotorolaSystem::handleBit(bool bDataBit)
{
  //
  // keep up 8 bit sliding register for sync checking purposes
  //
  m_nShiftReg = (m_nShiftReg << 1) & 0xff;

  if (bDataBit) 
    m_nShiftReg = m_nShiftReg | 0x01;

  // this was "m_bs-1" which could have -1 as an array index!
  m_ob[m_bs++] = bDataBit;

  //
  //  If sync seq and enough bits are found - data block
  //
  if (m_nShiftReg == m_fs && m_bs > 83)
  {
    //
    // The original code didn't handle excess bits at beginning. Not
    // sure it will make much difference, but I send the last 84 bits
    // out, not the first 84 bits.  The latest frame is stored in array 
    // m_ob[].
    //
    pigout(m_bs - 84);  
    m_bs = 0;
    m_bInSync = true;
  }

  // handle streaming garbage while waiting for sync bits to appear.
  if (m_bs > 98)
  {
	// shift up - throw away the oldest 15 or so bits.
	for (int i = 0; i < 84; i++)
      m_ob[i] = m_ob[i + m_bs - 84];
	// TODO: recode this to be a ring buffer
    m_bs = 84;
    //
    //  High-precision receiver status
    //
    m_bInSync = false; 
  }
}

unsigned short idMask[14] = {0};
unsigned short radioMask[14] = {0};
