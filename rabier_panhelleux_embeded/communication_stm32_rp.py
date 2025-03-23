import serial
import serial.tools.list_ports
import numpy as np

# Indiquez le port série à utiliser (adapter selon votre configuration)
PORT = "COM8"

# Nombre de classes de sortie de votre nouveau réseau
NB_CLASSES = 5

def synchroniser_UART(serial_port):
    """
    Synchronise la communication UART en envoyant un octet particulier
    (0xAB) et en attendant la réponse (0xCD).
    """
    while True:
        serial_port.write(b"\xAB")  # Envoi de l'octet 0xAB
        reponse = serial_port.read(1)
        if reponse == b"\xCD":
            # Lecture d'un éventuel octet supplémentaire si nécessaire
            serial_port.read(1)
            break

def envoyer_entrees_au_STM32(inputs, serial_port):
    """
    Envoie un tableau numpy d'entrées (floats 32 bits) vers la carte STM32 via UART.
    """
    inputs = inputs.astype(np.float32)
    buffer = b""
    for val in inputs:
        buffer += val.tobytes()  # Convertit chaque float32 en bytes
    serial_port.write(buffer)

def lire_sorties_du_STM32(serial_port):
    """
    Lit NB_CLASSES octets depuis le port série et les convertit en floats
    (chacun étant un entier 0-255, normalisé par 255).
    """
    output_bytes = serial_port.read(NB_CLASSES)
    # Convertit chaque octet en float entre 0.0 et 1.0
    float_values = [int(o) / 255.0 for o in output_bytes]
    return float_values

def evaluer_modele_sur_STM32(iterations, serial_port):
    """
    Évalue la précision du modèle sur la STM32 en envoyant 'iterations' échantillons.
    On compare la classe prédite (argmax) à la classe réelle (argmax de Y_test).
    """
    accuracy = 0.0
    for i in range(iterations):
        print(f"----- Itération {i+1} -----")
        # Envoi de l'échantillon X_test[i]
        envoyer_entrees_au_STM32(X_test[i], serial_port)

        # Lecture de la sortie (probabilités pour les 5 classes)
        output = lire_sorties_du_STM32(serial_port)

        # Vérifie si la classe prédite (argmax) est la même que la classe attendue
        if np.argmax(output) == np.argmax(Y_test[i]):
            accuracy += 1.0 / iterations

        print(f"   Classe attendue : {np.argmax(Y_test[i])}")
        print(f"   Classe prédite  : {np.argmax(output)}")
        print(f"   Sortie brute    : {output}")
        print(f"Précision cumulée : {accuracy:.2f}\n")

    return accuracy

if __name__ == '__main__':
    # Chargez vos fichiers NumPy (adapter les chemins et noms de fichiers si besoin)
    X_test = np.load("./X_valid_resized_mini.npy")  # ex. shape: (N, input_dim)
    Y_test = np.load("./Y_valid_resized_mini.npy")  # ex. shape: (N, 5) en one-hot

    # Ouvrir le port série pour communiquer avec la STM32
    with serial.Serial(PORT, 115200, timeout=1) as ser:
        print("Synchronisation en cours...")
        synchroniser_UART(ser)
        print("Synchronisation réussie.")

        print("Évaluation du modèle sur STM32...")
        nb_iterations = 100  # Nombre d'échantillons à tester
        accuracy = evaluer_modele_sur_STM32(nb_iterations, ser)
        print(f"Précision finale : {accuracy * 100:.2f}%")
