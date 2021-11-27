package pl.nieruchalski.client.domain.subscriber;

import pl.nieruchalski.client.domain.values.event.ViewerHost;

public interface NewConnectionSubscriber {
    void handleNewConnection(ViewerHost viewerHost);
}
