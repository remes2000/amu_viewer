package pl.nieruchalski.client.domain.service;

import pl.nieruchalski.client.domain.exception.EstablishConnectionException;
import pl.nieruchalski.client.domain.exception.HostRefusedAccessCodeException;
import pl.nieruchalski.client.domain.values.ConnectionRequest;
import pl.nieruchalski.client.domain.values.ViewerHost;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.HashSet;
import java.util.Set;

public class ConnectionService {
    private static ConnectionService service;
    private Set<ViewerHost> activeHosts = new HashSet();

    public static ConnectionService getInstance() {
        if(service == null) {
            service = new ConnectionService();
        }
        return service;
    }

    private ConnectionService() {}

    public void establishConnection(ConnectionRequest connectionRequest) throws EstablishConnectionException {
        try {
            Socket socket = new Socket(connectionRequest.getIp(), connectionRequest.getPort());
            DataOutputStream outputStream = new DataOutputStream(socket.getOutputStream());
            outputStream.writeInt(connectionRequest.getAccessCode());
        } catch (IOException e) {
            e.printStackTrace();
            throw new EstablishConnectionException();
        }
    }
}
