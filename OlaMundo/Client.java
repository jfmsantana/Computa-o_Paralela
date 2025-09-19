import java.net.MalformedURLException;
import java.rmi.Naming;
import java.rmi.RemoteException;
import java.rmi.NotBoundException;

public class Client {
    public static void main(String[] args) {
        try {

            Ola stub = (Ola) Naming.lookup("rmi://localhost/Ola");
            System.out.println(stub.digaOla("Santana"));

        } catch (RemoteException | MalformedURLException | NotBoundException e) {

            e.printStackTrace();
        }
    }
}

// javc *.java
// java Server
// DEpois, em um novo terminal, rodar "java Client"