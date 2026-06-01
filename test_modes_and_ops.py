import socket
import time

SERVER = "127.0.0.1"
PORT = 6667
PASSWORD = "mdp"

def send_msg(sock, msg):
    sock.send((msg + "\r\n").encode())
    time.sleep(0.1)

def get_response(sock):
    sock.settimeout(1.0)
    try:
        return sock.recv(4096).decode()
    except socket.timeout:
        return ""

def test_modes_and_ops():
    print("--- Démarrage des tests Modes & Opérateurs ---")

    # Connexion de l'Opérateur (Alice)
    op_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    op_sock.connect((SERVER, PORT))
    send_msg(op_sock, f"PASS {PASSWORD}")
    send_msg(op_sock, "NICK Alice")
    send_msg(op_sock, "USER Alice 0 * :Alice OP")

    # Connexion du Client normal (Bob)
    client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_sock.connect((SERVER, PORT))
    send_msg(client_sock, f"PASS {PASSWORD}")
    send_msg(client_sock, "NICK Bob")
    send_msg(client_sock, "USER Bob 0 * :Bob Normal")

    get_response(op_sock)
    get_response(client_sock)

    # 1. Test de KICK
    print("[TEST] Création du salon et KICK")
    send_msg(op_sock, "JOIN #fortress")
    send_msg(client_sock, "JOIN #fortress")
    send_msg(op_sock, "KICK #fortress Bob :Dehors !")
    resp = get_response(client_sock)
    assert "KICK" in resp and "Bob" in resp, "Échec: Bob n'a pas été kické."
    print("✅ KICK réussi")

    # 2. Test du Mode +k (Mot de passe)
    print("[TEST] Mode +k (Mot de passe)")
    send_msg(op_sock, "MODE #fortress +k secret")
    send_msg(client_sock, "JOIN #fortress") # Sans mot de passe
    resp = get_response(client_sock)
    assert "475" in resp, "Échec: Bob a pu rejoindre sans mot de passe."
    send_msg(client_sock, "JOIN #fortress secret") # Avec mot de passe
    resp = get_response(client_sock)
    assert "JOIN" in resp, "Échec: Bob n'a pas pu rejoindre avec le bon mot de passe."
    print("✅ Mode +k réussi")

    # 3. Test de TOPIC (Mode +t)
    print("[TEST] Commande TOPIC et Mode +t")
    send_msg(op_sock, "MODE #fortress +t")
    send_msg(client_sock, "TOPIC #fortress :Nouveau sujet")
    resp = get_response(client_sock)
    assert "482" in resp, "Échec: Bob a pu changer le topic alors qu'il n'est pas OP."
    send_msg(op_sock, "TOPIC #fortress :Sujet valide")
    resp = get_response(client_sock)
    assert "TOPIC" in resp and "Sujet valide" in resp, "Échec: Alice (OP) n'a pas pu changer le topic."
    print("✅ Commande TOPIC réussie")

    # 4. Test du Mode +i et INVITE
    print("[TEST] Mode +i et INVITE")
    send_msg(op_sock, "KICK #fortress Bob :Out again")
    send_msg(op_sock, "MODE #fortress +i")
    send_msg(client_sock, "JOIN #fortress secret") # Tente de rejoindre, mais c'est sur invitation
    resp = get_response(client_sock)
    assert "473" in resp, "Échec: Bob a rejoint un canal +i sans invitation."
    send_msg(op_sock, "INVITE Bob #fortress")
    send_msg(client_sock, "JOIN #fortress secret")
    resp = get_response(client_sock)
    assert "JOIN" in resp, "Échec: Bob n'a pas pu rejoindre après invitation."
    print("✅ Mode +i et INVITE réussis")

    op_sock.close()
    client_sock.close()
    print("--- Tous les tests Modes & Opérateurs ont réussi ! ---\n")

if __name__ == "__main__":
    test_modes_and_ops()