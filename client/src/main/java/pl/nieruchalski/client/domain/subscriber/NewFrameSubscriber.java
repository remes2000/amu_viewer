package pl.nieruchalski.client.domain.subscriber;

import pl.nieruchalski.client.domain.values.event.Frame;

public interface NewFrameSubscriber {
    void handleNewFrame(Frame frame);
}
