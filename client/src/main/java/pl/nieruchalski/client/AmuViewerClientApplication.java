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
        Scene scene = new Scene(fxmlLoader.load());
        EventBinder.getInstance().setScene(scene);
        EventBinder.getInstance().bind();
        stage.setTitle("amu_viewer");
        stage.setScene(scene);
        stage.setWidth(VIEWER_WIDTH);
        stage.setHeight(VIEWER_HEIGHT);
        stage.show();
        stage.setOnCloseRequest(e -> {
            ConnectionService.getInstance().closeAllConnectionsSilent();
        });
    }

    public static void main(String[] args) {
        launch();
    }
}