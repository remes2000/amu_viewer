package pl.nieruchalski.client.domain.publisher;

import pl.nieruchalski.client.domain.publisher.base.BasePublisher;
import pl.nieruchalski.client.domain.subscriber.GeneralSubscriber;
import pl.nieruchalski.client.domain.values.event.general.GeneralInformation;
import pl.nieruchalski.client.domain.values.event.general.UdpPortChange;

public class GeneralPublisher extends BasePublisher<GeneralInformation, GeneralSubscriber> {
    private static GeneralPublisher publisher;

    private GeneralPublisher() {}

    public static GeneralPublisher getInstance() {
        if(publisher == null) {
            publisher = new GeneralPublisher();
        }
        return publisher;
    }

    @Override
    protected void notify(GeneralSubscriber generalSubscriber, GeneralInformation generalInformation) {
        if(generalInformation instanceof UdpPortChange) {
            generalSubscriber.handleUdpPortChange((UdpPortChange) generalInformation);
        }
    }
}
