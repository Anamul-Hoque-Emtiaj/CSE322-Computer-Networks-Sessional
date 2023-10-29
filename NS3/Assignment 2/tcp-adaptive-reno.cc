#include "tcp-adaptive-reno.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpAdaptiveReno");
NS_OBJECT_ENSURE_REGISTERED(TcpAdaptiveReno);

TypeId
TcpAdaptiveReno::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpAdaptiveReno")
            .SetParent<TcpWestwoodPlus>()
            .SetGroupName("Internet")
            .AddConstructor<TcpAdaptiveReno>();
            /*.AddAttribute("FilterType",
                          "Use this to choose no filter or Tustin's approximation filter",
                          EnumValue(TcpAdaptiveReno::TUSTIN),
                          MakeEnumAccessor(&TcpAdaptiveReno::m_fType),
                          MakeEnumChecker(TcpAdaptiveReno::NONE, "None", TcpAdaptiveReno::TUSTIN, "Tustin"))
            .AddTraceSource("EstimatedBW",
                            "The estimated bandwidth",
                            MakeTraceSourceAccessor(&TcpAdaptiveReno::m_currentBW),
                            "ns3::TracedValueCallback::DataRate");*/
    return tid;
}

TcpAdaptiveReno::TcpAdaptiveReno()
    : TcpWestwoodPlus(),
      incWnd(0),
      baseWnd(0),
      probeWnd(0),
      minRtt(Seconds(0)),
      currentRtt(Seconds(0)),
      prevConjRtt(Seconds(0)),
      conjRtt(Seconds(0)),
      jPacketLRtt(Seconds(0))
{
    NS_LOG_FUNCTION(this);
}

TcpAdaptiveReno::TcpAdaptiveReno(const TcpAdaptiveReno& sock)
    : TcpWestwoodPlus(sock),
      incWnd(sock.incWnd),
      baseWnd(sock.baseWnd),
      probeWnd(sock.probeWnd),
      minRtt(sock.minRtt),
      currentRtt(sock.currentRtt),
      prevConjRtt(sock.prevConjRtt),
      conjRtt(sock.conjRtt),
      jPacketLRtt(sock.jPacketLRtt)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_LOGIC("Invoked the copy constructor");
}

TcpAdaptiveReno::~TcpAdaptiveReno()
{
}

double TcpAdaptiveReno::EstimateCongestionLevel()
{
    double a = 0.85; // exponential smoothing factor
    if (prevConjRtt < minRtt)
        a = 0; 

    double conjRttValue = a * prevConjRtt.GetSeconds() + (1 - a) * jPacketLRtt.GetSeconds();
    conjRtt = Seconds(conjRttValue);

    return std::min(
        (currentRtt.GetSeconds() - minRtt.GetSeconds()) / (conjRttValue - minRtt.GetSeconds()),
        1.0
    );
}

void TcpAdaptiveReno::EstimateIncWnd(Ptr<TcpSocketState> tcb)
{
    double congestion = EstimateCongestionLevel();
    int M = 1000; // 10 mbps in paper
    double MSS = static_cast<double>(tcb->m_segmentSize * tcb->m_segmentSize);

    double maxIncWnd = static_cast<double>(m_currentBW.Get().GetBitRate()) / (M * MSS);

    double alpha = 10; 
    double beta = 2 * maxIncWnd * ((1 / alpha) - ((1 / alpha + 1) / (std::exp(alpha))));
    double gamma = 1 - (2 * maxIncWnd * ((1 / alpha) - ((1 / alpha + 0.5) / (std::exp(alpha)))));

    incWnd = static_cast<int>((maxIncWnd / std::exp(alpha * congestion)) + (beta * congestion) + gamma);
}

void TcpAdaptiveReno::CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
    if (segmentsAcked > 0)
    {
        EstimateIncWnd(tcb);

        double adder = static_cast<double>(tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get();
        adder = std::max(1.0, adder);
        baseWnd += static_cast<uint32_t>(adder);

        probeWnd = std::max(
            static_cast<double>(probeWnd + incWnd / (int)tcb->m_cWnd.Get()),
            static_cast<double>(0)
        );

        tcb->m_cWnd = baseWnd + probeWnd;
    }
}

void TcpAdaptiveReno::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt)
{
    NS_LOG_FUNCTION(this << tcb << packetsAcked << rtt);

    if (rtt.IsZero())
    {
        NS_LOG_WARN("RTT measured is zero!");
        return;
    }

    m_ackedSegments += packetsAcked;
    currentRtt = rtt;

    if (minRtt.IsZero())
        minRtt = rtt;
    else if (rtt <= minRtt)
        minRtt = rtt;

    TcpWestwoodPlus::EstimateBW(rtt, tcb);
}

uint32_t TcpAdaptiveReno::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight)
{
    prevConjRtt = conjRtt;
    jPacketLRtt = currentRtt;

    double congestion = EstimateCongestionLevel();

    uint32_t ssthresh = std::max(
        2 * tcb->m_segmentSize,
        static_cast<uint32_t>(tcb->m_cWnd / (1.0 + congestion))
    );

    baseWnd = ssthresh;
    probeWnd = 0;

    return ssthresh;
}

} // namespace ns3
