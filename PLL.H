#ifndef _PLL_H
#define _PLL_H

class CPhaseLockedLoop
{
private:
  const long m_kSamplesPerSec;
  const long m_kBPS;
  const double m_kBitTime;
  const double m_kBitHalfTime;

  long m_nAccumulate;

  double m_fAccumulate;
  double m_fPhase;

  bool m_bSense;
  bool m_bLocked;
  bool m_bInvert;

protected:
   void setLocked(bool bLocked) { m_bLocked = bLocked; };
   // must be provided by subclass
   virtual void handleBit(bool bDataBit) = 0;

public:
  CPhaseLockedLoop(const long nBPS, bool bSense, const long nSamplesPerSec) :
	m_kSamplesPerSec(nSamplesPerSec),
    m_kBPS(nBPS),
    m_kBitTime(1.0 / nBPS),
    m_kBitHalfTime(0.5 / nBPS),

	m_nAccumulate(0),
    m_fAccumulate(0.0),
    m_fPhase(m_kBitHalfTime),

    m_bSense(bSense),
    m_bLocked(false),
	m_bInvert(false)
  {
  };
  virtual ~CPhaseLockedLoop() {};

  bool isLocked() const { return m_bLocked; };
  void setInvert(bool bInvert) { m_bInvert = bInvert; };
  bool getInvert(void) const { return m_bInvert; };

  void inputData(bool bDataBit, long nDelta)
  {
	double fDelta = nDelta;
	fDelta /= m_kSamplesPerSec;
    m_fAccumulate += fDelta; // + m_fPhase;
	bDataBit ^= m_bInvert;
    while ( m_fAccumulate >= m_kBitHalfTime)
    {
      handleBit(bDataBit);
      m_fAccumulate -= m_kBitTime;
    }
	m_fPhase = m_fAccumulate;
	//printf("%f\r", m_fPhase);
   };
};

#endif
