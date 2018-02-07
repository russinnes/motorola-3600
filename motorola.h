#ifndef _MOTOROLA_H
#define _MOTOROLA_H

#include "PLL.h"

#include <time.h>

struct OSW
{
  unsigned short cmd;
  unsigned short id;
  bool grp;
  bool isFreq;
};

extern unsigned short idMask[14];
extern unsigned short radioMask[14];

#define OTHER_DIGITAL 16
#define PHONE_PATCH 32

#define OSW_BACKGROUND_IDLE     0x02f8
#define OSW_FIRST_CODED_PC      0x0304
#define OSW_FIRST_NORMAL        0x0308
#define OSW_FIRST_TY2AS1        0x0309
#define OSW_EXTENDED_FCN        0x030b                      
#define OSW_AFFIL_FCN           0x030d
#define OSW_TY2_AFFILIATION     0x0310
#define OSW_TY1_STATUS_MIN      0x0310
#define OSW_TY2_MESSAGE         0x0311
#define OSW_TY1_STATUS_MAX      0x0317
#define OSW_TY1_ALERT           0x0318
#define OSW_TY1_EMERGENCY       0x0319
#define OSW_TY2_CALL_ALERT      0x0319
#define OSW_FIRST_ASTRO         0x0321
#define OSW_SYSTEM_CLOCK        0x0322
#define OSW_SCAN_MARKER         0x032b
#define OSW_EMERG_ANNC          0x032e
#define OSW_AMSS_ID_MIN         0x0360
#define OSW_AMSS_ID_MAX         0x039f
#define OSW_CW_ID               0x03a0
#define OSW_SYS_NETSTAT         0x03bf
#define OSW_SYS_STATUS          0x03c0

class CMotorolaSystem : public CPhaseLockedLoop
{
  // raw incoming OSW bits (stage zero)
  bool m_ob[100];
  // "good" incoming OSW bits (stage one)
  bool m_gob[100];
  // corrected, de-interleaved OSW bits (stage two)
  bool m_osw[50];
  // the current OSW (final stage)
  struct OSW m_bosw;
  // counter/index into m_gob array.
  unsigned m_ct;
  // number of bits in the m_ob array.
  int m_bs;
  // counts number of bad OSWs
  unsigned m_nBadCount;
  // counts number of good OSWs
  unsigned m_nGoodCount;
  // true if currently receiving consecutive 
  // frames without unrecoverable errors.
  bool m_bInSync;
  // eight bit shift register - used to find frame sync.
  int m_nShiftReg;
  // frame sync value
  const int m_fs;

  // TBD?
  unsigned int ott1, ott2;

  bool m_bIsNetworkable;
  bool m_bIsNetworked;
  unsigned short m_nNoteSite;
  // the OSW stack
  struct OSW stack[5];
  // stack depth
  int numStacked;
  int idlesWithNoId;
  int osw_state;
  int netCounts;
  int numConsumed;
  bool m_bVerbose;
  int m_nSiteId;
  int m_nSysId;
  bool m_bVhfUhf;
  time_t lastT;
  time_t now;
  time_t nowFine;

protected:
  virtual void handleBit(bool bDataBit);

public:
  CMotorolaSystem(long nBPS, bool bSense, long nSamplesPerSecond) : CPhaseLockedLoop(nBPS, bSense, nSamplesPerSecond), 
	  m_nShiftReg(0), m_fs(0xac), m_bs(0), m_ct(0), 
	  m_nBadCount(0), m_nGoodCount(0), m_bInSync(false),
	  m_bIsNetworkable(false), m_nNoteSite(0), numStacked(0),
	  idlesWithNoId(0), netCounts(0), numConsumed(0),
	  m_bVerbose(false), m_bIsNetworked(false), m_nSiteId(0),
	  m_nSysId(1234), m_bVhfUhf(false), osw_state(eIdentifying)
  {
	int i = 0;
	for (i = 0; i < sizeof m_ob; i++) m_ob[i] = 0;
	for (i = 0; i < sizeof m_gob; i++) m_gob[i] = 0;
	for (i = 0; i < sizeof m_osw; i++) m_osw[i] = 0;

    time(&lastT);
    time(&now);
    time(&nowFine);
  };

private:
  void pigout(int nSkipOver);
  void process_osw(bool sl);
  void show_good_osw (struct OSW &osw);
  void twoOSWcall(unsigned short flags = 0);

  enum eStates {eIdentifying, eOperNewer, eGettingOldId, eOperOlder};
};

#endif
