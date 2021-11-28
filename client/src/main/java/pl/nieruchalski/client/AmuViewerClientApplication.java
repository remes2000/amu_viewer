package pl.nieruchalski.client;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.stage.Stage;
import pl.nieruchalski.client.domain.service.ConnectionService;

import java.io.IOException;

public class AmuViewerClientApplication extends Application {
    public static Integer VIEWER_WIDTH = 1200;
    public static Integer VIEWER_HEIGHT = 700;

    @Override
    public void start(Stage stage) throws IOException {
        FXMLLoader fxmlLoader = new FXMLLoader(AmuViewerClientApplication.class.getResource("amu-viewer-view.fxml"));
        Scene scene = new Scene(fxmlLoader.load(), VIEWER_WIDTH, VIEWER_HEIGHT);
        stage.setTitle("amu_viewer");
        stage.setScene(scene);
        stage.show();
    }

    public static void main(String[] args) {
        launch();
    }
}