package pl.nieruchalski.client.domain.publisher;

import pl.nieruchalski.client.domain.publisher.base.BasePublisher;
import pl.nieruchalski.client.domain.subscriber.NewFrameSubscriber;
import pl.nieruchalski.client.domain.values.event.Frame;

public class NewFramePublisher extends BasePublisher<Frame, NewFrameSubscriber> {
    private static NewFramePublisher publisher;

    private NewFramePublisher() {}

    public static NewFramePublisher getInstance() {
        if(publisher == null) {
            publisher = new NewFramePublisher();
        }
        return publisher;
    }

    @Override
    protected void notify(NewFrameSubscriber newFrameSubscriber, Frame frame) {
        newFrameSubscriber.handleNewFrame(frame);
    }
}
