import jakarta.websocket.DeploymentException;
import org.glassfish.tyrus.server.Server;

import java.util.Collections;

public class SynthBridge {

    public static void main(String[] args) {
        Server server = new Server("localhost", 8080, "/",
                Collections.emptyMap(), WebSocketsEndpoint.class);
        try {
            server.start();
            System.out.println("WebSocket server started at ws://localhost:8080/signal");
            while (true) {

            }
        } catch (DeploymentException e) {
            throw new RuntimeException(e);
        }
    }

}
