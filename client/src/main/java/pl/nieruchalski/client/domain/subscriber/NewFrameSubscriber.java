package pl.nieruchalski.client.domain.subscriber;

import pl.nieruchalski.client.domain.values.event.Frame;
import pl.nieruchalski.client.domain.values.event.ViewerHost;

public interface NewFrameSubscriber {
    void handleNewFrame(ViewerHost host);
}
