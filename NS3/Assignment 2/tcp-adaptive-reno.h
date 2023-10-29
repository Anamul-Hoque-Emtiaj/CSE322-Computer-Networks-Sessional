#ifndef TCP_ADAPTIVE_RENO_H
#define TCP_ADAPTIVE_RENO_H

#include "tcp-westwood-plus.h"

namespace ns3 {

class TcpAdaptiveReno : public TcpWestwoodPlus {
public:
    static TypeId GetTypeId();

    TcpAdaptiveReno();
    TcpAdaptiveReno(const TcpAdaptiveReno& sock);
    ~TcpAdaptiveReno() override;

    uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight) override;
    void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt) override;
    void CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;

protected:

    int32_t incWnd; //increase of window for the window probe part
    uint32_t baseWnd; //base part of the calculated congestion window
    uint32_t probeWnd; //probe part of the calculated congestion window 

    Time minRtt; // Minimum RTT
    Time currentRtt; // Current RTT
    Time prevConjRtt; // congestion Rtt for (j-1)-th packet loss event
    Time conjRtt; //  congestion Rtt for j-th packet loss event
    Time jPacketLRtt; // Rtt for j-th packet loss event

    

private:
    void EstimateIncWnd(Ptr<TcpSocketState> tcb);
    double EstimateCongestionLevel();
};

} // namespace ns3

#endif /* TCP_ADAPTIVE_RENO_H */
