package pl.nieruchalski.client.domain.exception;

public class InvalidConnectionRequestDataException extends RuntimeException {
    public InvalidConnectionRequestDataException(String message) {
        super(message);
    }
}
