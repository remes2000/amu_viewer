package pl.nieruchalski.client.domain.publisher.base;

import java.util.ArrayList;
import java.util.List;

public abstract class BasePublisher<TEvent, TSubscriber> {
    protected List<TSubscriber> subscribers = new ArrayList();

    public void subscribe(TSubscriber subscriber) {
        this.subscribers.add(subscriber);
    }

    public void broadcast(TEvent event) {
        for(TSubscriber subscriber : subscribers) {
            notify(subscriber, event);
        }
    }

    protected abstract void notify(TSubscriber subscriber, TEvent event);
}
