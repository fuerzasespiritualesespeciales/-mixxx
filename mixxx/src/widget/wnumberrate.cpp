//
// C++ Implementation: wnumberrate
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <math.h>
#include "wnumberrate.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"

WNumberRate::WNumberRate(const char * group, QWidget * parent) : WNumber(parent)
{
    m_pRateRangeControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rateRange")));
    m_pRateDirControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rate_dir")));
    m_pRateControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rate")));
}

WNumberRate::~WNumberRate()
{
    delete m_pRateControl;
    delete m_pRateDirControl;
    delete m_pRateRangeControl;
}

void WNumberRate::setValue(double)
{
    double vsign = m_pRateControl->get()*m_pRateRangeControl->get()*m_pRateDirControl->get();

    char sign = '+';
    if (vsign < -0.00000001) {
        sign = '-';
    }

    m_pLabel->setText(QString(m_qsText).append(sign).append("%1").arg(fabs(vsign)*100., 0, 'f', 2));
}

