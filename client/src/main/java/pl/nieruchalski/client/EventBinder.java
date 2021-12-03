package pl.nieruchalski.client;

import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.input.MouseButton;
import pl.nieruchalski.client.domain.service.HostManager;
import pl.nieruchalski.client.domain.values.event.ViewerHost;

public class EventBinder {
    private static EventBinder binder;

    public static EventBinder getInstance() {
        if(binder == null) {
            binder = new EventBinder();
        }
        return binder;
    }

    private Canvas display;
    private Scene scene;

    private EventBinder() {}

    public void setScene(Scene scene) {
        this.scene = scene;
    }

    public void setDisplay(Canvas display) {
        this.display = display;
    }

    public void bind() {
        this.bindMouseMove();
        this.bindMouseClick();
        this.bindKeyStroke();
    }

    private void bindMouseMove() {
        this.display.setOnMouseMoved(e -> {
            if(!this.isHostSelected()) {
                return;
            }
            this.getHost().mouseMove(toInteger(e.getX()), toInteger(e.getY()));
            e.consume();
        });
    }

    private void bindMouseClick() {
        this.display.setOnMousePressed(e -> {
            if(!this.isHostSelected()) {
                return;
            }
            this.display.requestFocus();
            if(e.getButton() == MouseButton.PRIMARY) {
                this.getHost().mouseLeftPressed(toInteger(e.getX()), toInteger(e.getY()));
            } else if(e.getButton() == MouseButton.SECONDARY) {
                this.getHost().mouseRightPressed(toInteger(e.getX()), toInteger(e.getY()));
            }
            e.consume();
        });
        this.display.setOnMouseReleased(e -> {
            if(!this.isHostSelected()) {
                return;
            }
            if(e.getButton() == MouseButton.PRIMARY) {
                this.getHost().mouseLeftReleased(toInteger(e.getX()), toInteger(e.getY()));
            } else if(e.getButton() == MouseButton.SECONDARY) {
                this.getHost().mouseRightReleased(toInteger(e.getX()), toInteger(e.getY()));
            }
            e.consume();
        });
    }

    private void bindKeyStroke() {
        this.scene.onKeyPressedProperty().bind(this.display.onKeyPressedProperty());
        this.scene.onKeyReleasedProperty().bind(this.display.onKeyReleasedProperty());
        this.display.setOnKeyPressed(e -> {
            if(!this.isHostSelected()) {
                return;
            }
            this.getHost().keyPressed(e.getCode());
            e.consume();
        });
        this.display.setOnKeyReleased(e -> {
            if(!this.isHostSelected()) {
                return;
            }
            this.getHost().keyReleased(e.getCode());
            e.consume();
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
