import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

public class Olalmpl extends UnicastRemoteObject implements Ola{

    protected Olalmpl() throws RemoteException {
        super();
    }

    private static final long serialVersionUID = 1L;

    @Override
    public String digaOla(String nome) {
        return "Olá, " + nome + "!";
    }
}