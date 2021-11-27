package pl.nieruchalski.client;

import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.stage.Stage;
import pl.nieruchalski.client.domain.exception.EstablishConnectionException;
import pl.nieruchalski.client.domain.exception.HostRefusedAccessCodeException;
import pl.nieruchalski.client.domain.exception.InvalidConnectionRequestDataException;
import pl.nieruchalski.client.domain.service.ConnectionService;
import pl.nieruchalski.client.domain.values.ConnectionRequest;

public class NewConnectionController {
    @FXML
    private TextField ipAddress;

    @FXML
    private TextField port;

    @FXML
    private TextField accessCode;

    @FXML
    private Label errorMessage;

    private ConnectionService service = ConnectionService.getInstance();

    @FXML
    private void submit() {
       errorMessage.setText("");
       try {
           ConnectionRequest connectionRequest = new ConnectionRequest(ipAddress.getText(), port.getText(), accessCode.getText());
           service.establishConnection(connectionRequest);
           this.closeWindow();
       } catch (InvalidConnectionRequestDataException e) {
            errorMessage.setText(e.getMessage());
       } catch (EstablishConnectionException e) {
           errorMessage.setText("Establish connection failed");
       } catch (HostRefusedAccessCodeException e) {
           errorMessage.setText("Host refused access code");
       }
    }

    private void closeWindow() {
        Stage stage = (Stage) ipAddress.getScene().getWindow();
        stage.close();
    }
}
