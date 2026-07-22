# ⏳ Pending

Captura **trabajo conocido pero aún sin fecha** — el "lo haremos algún día pero
no ahora".

🎯 **Aquí entra**: deuda detectada, follow-ups de incidentes, feedback accionable,
"esto hay que hacerlo pero no hemos decidido cuándo".
🚫 **Aquí NO entra**: trabajo ya agendado a un milestone (eso va a Linear).

🪄 **Promoción**: ponerle un milestone a un pending lo convierte en task Linear
`BEE-XXXX` y se elimina automáticamente de este fichero.

---

## 📋 Cómo añadir un pending

Usa el skill `/pending` (recomendado). O copia este bloque al final del fichero:

```markdown
### ⏳ pending-NNN — [Título corto]

- 📅 **Fecha añadida**: YYYY-MM-DD
- 🏷️ **Tipo**: feat | fix | docs | refactor | chore | infra | security | test
- 🧭 **Trigger**: por qué se añadió (incidente, feedback, deuda)
- ⚙️ **Acción requerida**: qué hay que hacer concretamente
- 🚧 **Bloqueado por**: (si aplica) algo o alguien que retrasa
- 🚦 **Estado**: 🆕 Nuevo
```

### 🏷️ Tipos disponibles

| Tipo | Cuándo usarlo |
|------|---------------|
| `feat` | Funcionalidad nueva |
| `fix` | Bug fix |
| `docs` | Solo documentación |
| `refactor` | Refactor sin cambio de comportamiento |
| `chore` | Mantenimiento, deps, config |
| `infra` | Infraestructura, CI/CD |
| `security` | Cuestiones de seguridad |
| `test` | Solo tests |

### 🚦 Estados posibles

| Iconito | Estado | Significado |
|---------|--------|-------------|
| 🆕 | Nuevo | Recién capturado, sin triage |
| 🔍 | En triage | Decidiendo prioridad / scope |
| 📋 | Promovido | Ya es task Linear (`BEE-XXXX`) — debería haberse eliminado de aquí |
| 🚧 | Bloqueado | Esperando algo externo (especificar) |
| ❌ | No procede | Decidido no avanzar (apuntar el porqué) |

---

## 🗂️ Pendientes registrados

### ⏳ pending-001 — 📉 `BEEPING_GetConfidence` devuelve el valor sin normalizar

- 📅 **Fecha añadida**: 2026-07-22
- 🏷️ **Tipo**: fix
- 🧭 **Trigger**: encontrado durante el Human QA Checkpoint de BEE-92 (`beeping_flutter`). En pantalla salen confianzas **negativas** (`-31%`), imposibles según el contrato publicado.
- ⚙️ **Acción requerida**: en `src/BeepingCoreLib_api.cpp`,

  ```cpp
  // return (beeping->mDecoder->GetConfidence()/2.f)+0.5f;   ← normalización comentada
  float result = beeping->mDecoder->GetConfidence();          ← valor crudo
  ```

  La línea comentada mapeaba `[-1, 1]` → `[0, 1]`. Está desactivada y se devuelve el valor crudo, mientras `include/BeepingCoreLib_api.h` documenta *"Combined reception-quality score (0.0 poor — 1.0 ideal)"*.

  Decidir cuál de los dos es el contrato bueno y alinear el otro — **no basta con descomentar**: hay que confirmar que el rango real de `mDecoder->GetConfidence()` es `[-1, 1]` y revisar `GetConfidenceError` / `GetConfidenceNoise`, que documentan el mismo rango y pueden tener el mismo problema. Añadir test que fije el rango.

  Aguas arriba todo confía en el contrato documentado: `BeepingPayload.confidence` (Swift), el `confidence` del `platform_interface` en Dart (`[0.0, 1.0]`) y el `*100` del example.

- 🚧 **Bloqueado por**: nada
- 🚦 **Estado**: 🆕 Nuevo

### ⏳ pending-002 — 👻 El decoder emite falsos positivos sobre ruido

- 📅 **Fecha añadida**: 2026-07-22
- 🏷️ **Tipo**: fix
- 🧭 **Trigger**: encontrado durante el Human QA Checkpoint de BEE-92. Con la app en `listening…` y **sin emitir nada**, la lista `heard` se llena de entradas `000000000` con la confianza bailando.
- ⚙️ **Acción requerida**: investigar por qué el decoder emite tokens de fin (`-3`) sobre ruido ambiente. Descartado que sea de las capas de arriba: el callback de captura en `beeping-ios/BeepingC.mm` filtra correctamente y solo reenvía los tokens `-2` (inicio) y `-3` (fin); todo lo demás (`>= 0` parcial, `-1` sin datos) se ignora. Y nueve ceros son un payload base32 válido de 9 chars, así que ninguna validación aguas arriba lo descarta.

  Mirar el umbral de detección y si `GetDecodedData` debería devolver estado de fallo en vez de una cadena de ceros.

- 🚧 **Bloqueado por**: nada
- 🚦 **Estado**: 🆕 Nuevo

**Por qué importa**: si el decoder dispara sobre ruido, distinguir un payload real de la basura en el round-trip se vuelve difícil, y eso compromete la validación del QA humano. Pendiente de comprobar si con un beep real la señal domina lo suficiente.
