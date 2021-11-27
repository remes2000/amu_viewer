package pl.nieruchalski.client;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.stage.Modality;
import javafx.stage.Stage;

import java.io.IOException;

public class AmuViewerController {
    @FXML
    private TabPane tabPane;

    public void initialize() {
//        tabPane.getTabs().add(new Tab("First tab"));
    }

    @FXML
    protected void onHelloButtonClick() {
        tabPane.getTabs().add(new Tab("Tab 1"));
    }

    @FXML
    protected void openNewConnectionDialog(ActionEvent event) {
        try {
            Stage stage = new Stage();
            Parent root = FXMLLoader.load(AmuViewerClientApplication.class.getResource("new-connection-view.fxml"));
            stage.setScene(new Scene(root));
            stage.setTitle("Establishing new connection");
            stage.initModality(Modality.APPLICATION_MODAL);
            stage.show();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}