package pl.nieruchalski.client;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.image.Image;
import javafx.stage.Modality;
import javafx.stage.Stage;
import pl.nieruchalski.client.domain.exception.CannotCloseConnectionWithHostException;
import pl.nieruchalski.client.domain.publisher.GeneralPublisher;
import pl.nieruchalski.client.domain.publisher.NewConnectionPublisher;
import pl.nieruchalski.client.domain.publisher.NewFramePublisher;
import pl.nieruchalski.client.domain.service.ConnectionService;
import pl.nieruchalski.client.domain.service.HostManager;
import pl.nieruchalski.client.domain.subscriber.GeneralSubscriber;
import pl.nieruchalski.client.domain.subscriber.NewConnectionSubscriber;
import pl.nieruchalski.client.domain.subscriber.NewFrameSubscriber;
import pl.nieruchalski.client.domain.values.event.Frame;
import pl.nieruchalski.client.domain.values.event.ViewerHost;
import pl.nieruchalski.client.domain.values.event.general.GeneralInformation;
import pl.nieruchalski.client.domain.values.event.general.UdpPortChange;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;

public class AmuViewerController implements NewConnectionSubscriber, NewFrameSubscriber, GeneralSubscriber {
    @FXML
    private TabPane tabPane;

    @FXML
    private Canvas display;

    @FXML
    private ScrollPane displayContainer;

    @FXML
    private Label udpPortLabel;

    private Map<Tab, ViewerHost> tabHostMap = new HashMap();

    public void initialize() {
        NewConnectionPublisher.getInstance().subscribe(this);
        NewFramePublisher.getInstance().subscribe(this);
        GeneralPublisher.getInstance().subscribe(this);
        ConnectionService.getInstance();
        this.displayContainer.setMaxWidth(AmuViewerClientApplication.VIEWER_WIDTH);
        this.displayContainer.setMinWidth(AmuViewerClientApplication.VIEWER_WIDTH);
        this.clearCanvas();
        tabPane.getSelectionModel().selectedItemProperty().addListener((ov, oldTab, newTab) -> {
            ViewerHost selectedHost = this.tabHostMap.get(newTab);
            HostManager.getInstance().selectHost(selectedHost);
            if(selectedHost != null) {
                this.renderHost(selectedHost);
            } else {
                this.clearCanvas();
            }
        });
        EventBinder.bind(display);
    }

    @FXML
    protected void openNewConnectionDialog(ActionEvent event) {
        try {
            Stage stage = new Stage();
            Parent root = FXMLLoader.load(AmuViewerClientApplication.class.getResource("new-connection-view.fxml"));
            stage.setScene(new Scene(root));
            stage.setWidth(400);
            stage.setTitle("Establishing new connection");
            stage.initModality(Modality.APPLICATION_MODAL);
            stage.show();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void handleNewConnection(ViewerHost viewerHost) {
        Tab connectionTab = new Tab(viewerHost.getName());
        connectionTab.setOnCloseRequest(e -> {
            try {
                ConnectionService.getInstance().closeConnection(viewerHost);
                this.tabHostMap.remove(connectionTab);
            } catch (CannotCloseConnectionWithHostException exception) {
                e.consume();
            }
        });
        HostManager.getInstance().selectHost(viewerHost);
        this.tabHostMap.put(connectionTab, viewerHost);
        tabPane.getTabs().add(connectionTab);
        tabPane.getSelectionModel().select(connectionTab);
    }

    @Override
    public void handleNewFrame(ViewerHost host) {
        ViewerHost selectedHost = HostManager.getInstance().getSelectedHost();
        if(selectedHost != null && selectedHost.getId().equals(host.getId())) {
            this.renderHost(host);
        }
    }

    @Override
    public void handleUdpPortChange(UdpPortChange udpPortChange) {
        this.udpPortLabel.setText("UDP: " + udpPortChange.getPort());
    }

    private void renderHost(ViewerHost host) {
        if(host.getLastFrame() != null) {
            this.renderFrame(host.getLastFrame());
        } else {
            this.clearCanvas();
        }
    }

    private void renderFrame(Frame frame) {
        this.displayContainer.setVisible(true);
        Image image = new Image(new ByteArrayInputStream(frame.buffer));
        display.setWidth(image.getWidth());
        display.setHeight(image.getHeight());
        display.getGraphicsContext2D().drawImage(image, 0, 0, image.getWidth(), image.getHeight());
    }

    private void clearCanvas() {
        this.displayContainer.setVisible(false);
        this.display.setWidth(0);
        this.display.setHeight(0);
//        try {
//            Image image = new Image(new FileInputStream("test.png"));
//            display.setWidth(image.getWidth());
//            display.setHeight(image.getHeight());
//            display.getGraphicsContext2D().drawImage(image, 0, 0, image.getWidth(), image.getHeight());
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
    }
}