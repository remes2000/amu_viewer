package pl.nieruchalski.client.domain.publisher;

import pl.nieruchalski.client.domain.publisher.base.BasePublisher;
import pl.nieruchalski.client.domain.subscriber.NewConnectionSubscriber;
import pl.nieruchalski.client.domain.values.event.ViewerHost;

public class NewConnectionPublisher extends BasePublisher<ViewerHost, NewConnectionSubscriber> {
    private static NewConnectionPublisher publisher;

    private NewConnectionPublisher() {}

    public static NewConnectionPublisher getInstance() {
        if(publisher == null) {
            publisher = new NewConnectionPublisher();
        }
        return publisher;
    }

    @Override
    protected void notify(NewConnectionSubscriber subscriber, ViewerHost viewerHost) {
        subscriber.handleNewConnection(viewerHost);
    }
}
