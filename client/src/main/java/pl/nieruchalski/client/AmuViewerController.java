package pl.nieruchalski.client;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.control.Label;
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
import pl.nieruchalski.client.domain.subscriber.GeneralSubscriber;
import pl.nieruchalski.client.domain.subscriber.NewConnectionSubscriber;
import pl.nieruchalski.client.domain.subscriber.NewFrameSubscriber;
import pl.nieruchalski.client.domain.values.event.Frame;
import pl.nieruchalski.client.domain.values.event.ViewerHost;
import pl.nieruchalski.client.domain.values.event.general.UdpPortChange;

import java.io.*;

public class AmuViewerController implements NewConnectionSubscriber, NewFrameSubscriber, GeneralSubscriber {
    @FXML
    private TabPane tabPane;

    @FXML
    private Canvas display;

    @FXML
    private Label udpPortLabel;

    public void initialize() {
        NewConnectionPublisher.getInstance().subscribe(this);
        NewFramePublisher.getInstance().subscribe(this);
        GeneralPublisher.getInstance().subscribe(this);
        ConnectionService.getInstance();
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
            } catch (CannotCloseConnectionWithHostException exception) {
                e.consume();
            }
        });
        tabPane.getTabs().add(new Tab(viewerHost.getName()));
    }

    @Override
    public void handleNewFrame(ViewerHost host) {
        Image image = new Image(new ByteArrayInputStream(host.getLastFrame().buffer));
        display.setWidth(image.getWidth());
        display.setHeight(image.getHeight());
        display.getGraphicsContext2D().drawImage(image, 0, 0, image.getWidth(), image.getHeight());
    }

    @Override
    public void handleUdpPortChange(UdpPortChange udpPortChange) {
        this.udpPortLabel.setText("UDP: " + udpPortChange.getPort());
    }
}