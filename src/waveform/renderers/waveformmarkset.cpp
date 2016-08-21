#include <set>
#include <QtDebug>

#include "waveformmarkset.h"
#include "engine/cuecontrol.h"
#include "control/controlobject.h"
#include "util/memory.h"

WaveformMarkSet::WaveformMarkSet()
    : m_iFirstHotCue(-1) {
}

WaveformMarkSet::~WaveformMarkSet() {
    clear();
}

void WaveformMarkSet::setup(const QString& group, const QDomNode& node,
                            const SkinContext& context,
                            const WaveformSignalColors& signalColors) {

    clear();

#if QT_VERSION >= 0x040700
    m_marks.reserve(NUM_HOT_CUES);
#endif

    std::set<QString> controlItemSet;
    bool hasDefaultMark = false;

    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == "DefaultMark") {
            m_defaultMark.setup(group, child, context, signalColors);
            hasDefaultMark = true;
        } else if (child.nodeName() == "Mark") {
            m_marks.push_back(QSharedPointer<WaveformMark>(new WaveformMark()));
            WaveformMark* mark = m_marks.back().data();
            mark->setup(group, child, context, signalColors);

            if (mark->m_pPointCos) {
                // guarantee uniqueness even if there is a misdesigned skin
                QString item = mark->m_pPointCos->getKey().item;
                if (!controlItemSet.insert(item).second) {
                    qWarning() << "WaveformRenderMark::setup - redefinition of" << item;
                    m_marks.removeAt(m_marks.size() - 1);
                }
            }
        }
        child = child.nextSibling();
    }

    if (NUM_HOT_CUES >= 1) {
        m_iFirstHotCue = m_marks.size();
    }

    // check if there is a default mark and compare declared
    // and to create all missing hot_cues
    if (hasDefaultMark) {
        for (int i = 1; i <= NUM_HOT_CUES; ++i) {
            QString hotCueControlItem = "hotcue_" + QString::number(i) + "_position";
            ControlObject* pHotcue = ControlObject::getControl(
                    ConfigKey(group, hotCueControlItem));
            if (pHotcue == NULL) {
                continue;
            }

            if (controlItemSet.insert(hotCueControlItem).second) {
                //qDebug() << "WaveformRenderMark::setup - Automatic mark" << hotCueControlItem;
                QSharedPointer<WaveformMark> pMark(new WaveformMark(i));
                WaveformMarkProperties defaultProperties = m_defaultMark.getProperties();
                pMark->setProperties(defaultProperties);
                pMark->m_pPointCos = std::make_unique<ControlProxy>(pHotcue->getKey());
                m_marks.push_back(pMark);
            }
        }
    }
}

void WaveformMarkSet::clear() {
    m_defaultMark = WaveformMark();
    m_marks.clear();
}

QSharedPointer<WaveformMark> WaveformMarkSet::getHotCueMark(int hotCue) const {
    DEBUG_ASSERT(hotCue >= 0);
    DEBUG_ASSERT(hotCue < NUM_HOT_CUES);
    return operator[](m_iFirstHotCue + hotCue);
}

void WaveformMarkSet::setHotCueMark(int hotCue, QSharedPointer<WaveformMark> pMark) {
    m_marks[m_iFirstHotCue + hotCue] = pMark;
}
