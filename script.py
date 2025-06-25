import tkinter as tk

def binario_a_ascii(event=None):
    binario = entry.get("1.0", "end-1c")
    tecla_0 = entry_0.get()
    tecla_1 = entry_1.get()
    if not tecla_0 or not tecla_1 or tecla_0 == tecla_1:
        resultado.config(text="Error: Define teclas distintas para 0 y 1.")
        return
    try:
        binario_convertido = binario.replace(tecla_0, '0').replace(tecla_1, '1')
        binario_convertido = binario_convertido.replace(" ", "")
        caracteres = [binario_convertido[i:i+8] for i in range(0, len(binario_convertido), 8)]
        palabra = ''.join([chr(int(b, 2)) for b in caracteres if len(b) == 8])
        resultado.config(
            text=f"PALABRA: {palabra}\n({tecla_0}=0, {tecla_1}=1)"
        )
    except Exception:
        resultado.config(text="Error: Ingresa una cadena binaria válida.")

root = tk.Tk()
root.title("Binario a ASCII con teclas personalizadas")
root.geometry("800x350")
root.resizable(False, False)  # Ventana de tamaño fijo

fuente_grande = ("Arial", 18)
fuente_resultado = ("Arial", 24, "bold")

frame = tk.Frame(root)
frame.pack(pady=20)

tk.Label(frame, text="Tecla para 0:", font=fuente_grande).grid(row=0, column=0, padx=10)
entry_0 = tk.Entry(frame, width=3, font=fuente_grande, justify="center")
entry_0.insert(0, "0")
entry_0.grid(row=0, column=1, padx=10)
entry_0.bind("<KeyRelease>", binario_a_ascii)

tk.Label(frame, text="Tecla para 1:", font=fuente_grande).grid(row=0, column=2, padx=10)
entry_1 = tk.Entry(frame, width=3, font=fuente_grande, justify="center")
entry_1.insert(0, "1")
entry_1.grid(row=0, column=3, padx=10)
entry_1.bind("<KeyRelease>", binario_a_ascii)

tk.Label(root, text="Ingresa binario usando tus teclas elegidas:", font=fuente_grande).pack(pady=10)

text_frame = tk.Frame(root)
text_frame.pack()
entry = tk.Text(text_frame, width=48, height=4, font=fuente_grande, wrap="word", bd=4, relief="solid")
entry.pack()
entry.bind("<KeyRelease>", binario_a_ascii)

resultado = tk.Label(root, text="PALABRA: ", font=fuente_resultado, pady=15)
resultado.pack()

root.mainloop()