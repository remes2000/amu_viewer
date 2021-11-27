package pl.nieruchalski.client.domain.exception;

import java.net.SocketException;

public class CannotOpenUdpSocketException extends RuntimeException {
    public CannotOpenUdpSocketException(SocketException e) {
        super(e.getMessage());
    }
}
