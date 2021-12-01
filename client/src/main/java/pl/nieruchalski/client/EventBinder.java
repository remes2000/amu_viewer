package pl.nieruchalski.client;

import javafx.scene.canvas.Canvas;
import javafx.scene.input.MouseButton;
import pl.nieruchalski.client.domain.service.HostManager;
import pl.nieruchalski.client.domain.values.event.ViewerHost;

public class EventBinder {
    public static void bind(Canvas display) {
        new EventBinder(display);
    }

    private Canvas display;

    private EventBinder(Canvas display) {
        this.display = display;
//        this.bindMouseMove();
        this.bindMouseClick();
    }

    private void bindMouseMove() {
        this.display.setOnMouseMoved(e -> {
            if(!this.isHostSelected()) {
                return;
            }
            this.getHost().mouseMove(toInteger(e.getX()), toInteger(e.getY()));
        });
    }

    private void bindMouseClick() {
        this.display.setOnMouseClicked(e -> {
            if(!this.isHostSelected()) {
                return;
            }
            if(e.getButton() == MouseButton.PRIMARY) {
                this.getHost().mouseLeftClick(toInteger(e.getX()), toInteger(e.getY()));
            } else if(e.getButton() == MouseButton.SECONDARY) {
                this.getHost().mouseRightClick(toInteger(e.getX()), toInteger(e.getY()));
            }
        });
    }

    private boolean isHostSelected() {
        return HostManager.getInstance().getSelectedHost() != null;
    }

    private ViewerHost getHost() {
        return HostManager.getInstance().getSelectedHost();
    }

    private int toInteger(double d) {
        return Double.valueOf(d).intValue();
    }
}
