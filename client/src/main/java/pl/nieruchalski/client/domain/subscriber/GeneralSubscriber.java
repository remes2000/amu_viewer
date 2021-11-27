package pl.nieruchalski.client.domain.subscriber;

import pl.nieruchalski.client.domain.values.event.general.GeneralInformation;
import pl.nieruchalski.client.domain.values.event.general.UdpPortChange;

public interface GeneralSubscriber {
    void handleUdpPortChange(UdpPortChange udpPortChange);
}
