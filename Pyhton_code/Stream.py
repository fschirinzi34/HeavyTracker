import random

def crea_IP_Porta(num_pacchetti):

    lista_ip = []

    for i in range(num_pacchetti):
        a = random.randint(250, 258)
        b = random.randint(10, 100)
        ip_porta = f"192.168.{a}.{b}:80"
        lista_ip.append(ip_porta)
    return lista_ip

if __name__ == '__main__':
    lista_ip_porta = crea_IP_Porta(200000)

    f = open("Indirizzi_IP.txt","w")
    for i in lista_ip_porta:
        f.write(i)
        f.write("\n")



