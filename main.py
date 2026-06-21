import cv2
import numpy as np
import pandas as pd
from PIL import Image, ImageDraw, ImageFont

# Simulação de objetos detectados e rotas atribuídas
objetos = [
    {"id": "001", "tipo": "Carro", "rota": "1", "virada": "Direita"},
    {"id": "002", "tipo": "Moto", "rota": "22", "virada": "Reto"},
    {"id": "003", "tipo": "Caminhão", "rota": "17", "virada": "Esquerda"},
]

# Gera planilha Excel
df = pd.DataFrame(objetos)
df.to_excel("resultados/relatorio.xlsx", index=False)

# Carrega imagem base
imagem = Image.open("imagem_base/Ponto1.jpg")
draw = ImageDraw.Draw(imagem)

# Define cores por tipo
cores = {"Carro": "red", "Moto": "blue", "Caminhão": "green"}

# Simula rotas desenhadas
rotas_simuladas = {
    "1": [(430, 120), (400, 200), (380, 260)],
    "22": [(450, 100), (450, 200), (450, 300)],
    "17": [(460, 130), (500, 180), (530, 240)],
}

# Desenha linhas nas rotas
for obj in objetos:
    rota = obj["rota"]
    cor = cores.get(obj["tipo"], "black")
    pontos = rotas_simuladas[rota]
    draw.line(pontos, fill=cor, width=4)
    draw.text(pontos[-1], obj["id"], fill=cor)

# Salva imagem com rotas
imagem.save("resultados/mapa_rotas.jpg")
print("Relatório Excel e imagem gerados.")
