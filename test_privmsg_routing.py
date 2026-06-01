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

def test_privmsg():
    print("--- Démarrage des tests PRIVMSG ---")

    # Configuration de 3 clients
    socks = []
    names = ["Charlie", "David", "Eve"]

    for name in names:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((SERVER, PORT))
        send_msg(s, f"PASS {PASSWORD}")
        send_msg(s, f"NICK {name}")
        send_msg(s, f"USER {name} 0 * :{name}")
        get_response(s)
        socks.append(s)

    s_charlie, s_david, s_eve = socks

    # 1. Test Utilisateur vers Utilisateur
    print("[TEST] Message Utilisateur -> Utilisateur")
    send_msg(s_charlie, "PRIVMSG David :Salut David, c'est secret.")
    resp_david = get_response(s_david)
    assert "Salut David" in resp_david, "Échec: David n'a pas reçu le message privé."
    resp_eve = get_response(s_eve)
    assert "Salut David" not in resp_eve, "Échec: Eve a intercepté un message privé !"
    print("✅ Utilisateur -> Utilisateur réussi")

    # 2. Test Utilisateur Inexistant
    print("[TEST] Message vers cible introuvable")
    send_msg(s_charlie, "PRIVMSG Fantome :Tu es là ?")
    resp_charlie = get_response(s_charlie)
    assert "401" in resp_charlie, "Échec: Le serveur n'a pas signalé l'erreur 401 pour cible introuvable."
    print("✅ Cible introuvable gérée")

    # 3. Test Message de Groupe (Channel)
    print("[TEST] Message vers Channel")
    for s in socks:
        send_msg(s, "JOIN #chat_public")
        get_response(s) # Purge des messages de JOIN

    send_msg(s_charlie, "PRIVMSG #chat_public :Bonjour à tous !")

    resp_charlie = get_response(s_charlie)
    resp_david = get_response(s_david)
    resp_eve = get_response(s_eve)

    assert "Bonjour à tous" not in resp_charlie, "Échec: Charlie a reçu son propre message."
    assert "Bonjour à tous" in resp_david, "Échec: David n'a pas reçu le message du canal."
    assert "Bonjour à tous" in resp_eve, "Échec: Eve n'a pas reçu le message du canal."
    print("✅ Message Channel réussi")

    for s in socks:
        s.close()

    print("--- Tous les tests PRIVMSG ont réussi ! ---")

if __name__ == "__main__":
    test_privmsg()