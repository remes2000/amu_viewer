package pl.nieruchalski.client;

import javafx.fxml.FXML;
import javafx.scene.control.Label;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.control.TextField;
import pl.nieruchalski.client.domain.exception.EstablishConnectionException;
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
    protected void submit() {
       try {
           ConnectionRequest connectionRequest = new ConnectionRequest(ipAddress.getText(), port.getText(), accessCode.getText());
           service.establishConnection(connectionRequest);
       } catch (InvalidConnectionRequestDataException e) {
            errorMessage.setText(e.getMessage());
       } catch (EstablishConnectionException e) {

       }
    }
}
