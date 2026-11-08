{
  "patch": {
    "boxes": [
      {
        "box": {
          "id": "box-1",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "outlettype": [],
          "inlettype": ["int"],
          "patching_rect": [60, 40, 300, 24],
          "fontsize": 16,
          "fontname": "Arial",
          "text": "GyroSynth — Max for Live"
        }
      },
      {
        "box": {
          "id": "box-2",
          "maxclass": "live.toggle",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["int"],
          "inlettype": ["int"],
          "patching_rect": [60, 80, 50, 26],
          "parameter_enable": 0,
          "text": "ESP Serial"
        }
      },
      {
        "box": {
          "id": "box-3",
          "maxclass": "message",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [""],
          "inlettype": ["anything"],
          "patching_rect": [120, 80, 80, 20],
          "text": "port COM3"
        }
      },
      {
        "box": {
          "id": "box-4",
          "maxclass": "message",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [""],
          "inlettype": ["anything"],
          "patching_rect": [210, 80, 80, 20],
          "text": "baud 115200"
        }
      },
      {
        "box": {
          "id": "box-5",
          "maxclass": "message",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [""],
          "inlettype": ["anything"],
          "patching_rect": [300, 80, 60, 20],
          "text": "open"
        }
      },
      {
        "box": {
          "id": "box-6",
          "maxclass": "serial",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [""],
          "inlettype": ["anything"],
          "patching_rect": [60, 120, 140, 22]
        }
      },

      {
        "box": {
          "id": "box-7",
          "maxclass": "js",
          "numinlets": 1,
          "numoutlets": 3,
          "outlettype": ["float", "float", "float"],
          "inlettype": ["anything"],
          "patching_rect": [60, 160, 100, 22],
          "text": "parseGyro.js"
        }
      },
      {
        "box": {
          "id": "box-8",
          "maxclass": "number",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["int"],
          "inlettype": ["int"],
          "patching_rect": [60, 210, 50, 20],
          "fontsize": 12
        }
      },
      {
        "box": {
          "id": "box-9",
          "maxclass": "number",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["int"],
          "inlettype": ["int"],
          "patching_rect": [120, 210, 50, 20],
          "fontsize": 12
        }
      },
      {
        "box": {
          "id": "box-10",
          "maxclass": "number",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["int"],
          "inlettype": ["int"],
          "patching_rect": [180, 210, 50, 20],
          "fontsize": 12
        }
      },
      {
        "box": {
          "id": "box-11",
          "maxclass": "live.menu",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["int"],
          "inlettype": ["anything"],
          "patching_rect": [60, 250, 100, 26],
          "parameter_enable": 0,
          "text": "X",
          "items": ["X", "Y", "Z"]
        }
      },
      {
        "box": {
          "id": "box-12",
          "maxclass": "live.menu",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["int"],
          "inlettype": ["anything"],
          "patching_rect": [180, 250, 100, 26],
          "parameter_enable": 0,
          "text": "Частота",
          "items": ["Частота", "Амплитуда", "Фильтр"]
        }
      },
      {
        "box": {
          "id": "box-13",
          "maxclass": "live.dial",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["float"],
          "inlettype": ["float"],
          "patching_rect": [60, 300, 60, 60],
          "parameter_enable": 1,
          "text": "Базовая частота (Гц)"
        }
      },
      {
        "box": {
          "id": "box-14",
          "maxclass": "live.dial",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["float"],
          "inlettype": ["float"],
          "patching_rect": [140, 300, 60, 60],
          "parameter_enable": 1,
          "text": "Глубина LFO"
        }
      },
      {
        "box": {
          "id": "box-15",
          "maxclass": "gate",
          "numinlets": 2,
          "numoutlets": 3,
          "outlettype": ["int", "int", "int"],
          "inlettype": ["int", "int"],
          "patching_rect": [60, 380, 100, 20]
        }
      },
      {
        "box": {
          "id": "box-16",
          "maxclass": "line~",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["signal"],
          "inlettype": ["float"],
          "patching_rect": [60, 410, 80, 20]
        }
      },
      {
        "box": {
          "id": "box-17",
          "maxclass": "*~",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": ["signal"],
          "inlettype": ["signal", "signal"],
          "patching_rect": [60, 450, 40, 20]
        }
      },
      {
        "box": {
          "id": "box-18",
          "maxclass": "+~",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": ["signal"],
          "inlettype": ["signal", "signal"],
          "patching_rect": [60, 490, 40, 20]
        }
      },
      {
        "box": {
          "id": "box-19",
          "maxclass": "osc~",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": ["signal"],
          "inlettype": ["signal"],
          "patching_rect": [60, 530, 40, 20]
        }
      },
      {
        "box": {
          "id": "box-20",
          "maxclass": "*~",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": ["signal"],
          "inlettype": ["signal", "signal"],
          "patching_rect": [60, 570, 40, 20]
        }
      },
      {
        "box": {
          "id": "box-21",
          "maxclass": "plugoutput~",
          "numinlets": 2,
          "numoutlets": 0,
          "outlettype": [],
          "inlettype": ["signal", "signal"],
          "patching_rect": [60, 610, 80, 20]
        }
      },
      {
        "box": {
          "id": "box-22",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "outlettype": [],
          "inlettype": ["int"],
          "patching_rect": [60, 195, 40, 14],
          "fontsize": 10,
          "fontname": "Arial",
          "text": "X"
        }
      },
      {
        "box": {
          "id": "box-23",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "outlettype": [],
          "inlettype": ["int"],
          "patching_rect": [120, 195, 40, 14],
          "fontsize": 10,
          "fontname": "Arial",
          "text": "Y"
        }
      },
      {
        "box": {
          "id": "box-24",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "outlettype": [],
          "inlettype": ["int"],
          "patching_rect": [180, 195, 40, 14],
          "fontsize": 10,
          "fontname": "Arial",
          "text": "Z"
        }
      },
      {
        "box": {
          "id": "box-25",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "outlettype": [],
          "inlettype": ["int"],
          "patching_rect": [60, 280, 100, 14],
          "fontsize": 10,
          "fontname": "Arial",
          "text": "Источник угла"
        }
      },
      {
        "box": {
          "id": "box-26",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "outlettype": [],
          "inlettype": ["int"],
          "patching_rect": [180, 280, 80, 14],
          "fontsize": 10,
          "fontname": "Arial",
          "text": "Цель модуляции"
        }
      }
    ],
    "lines": [
      {"patchline": {"source": ["box-2", 0], "destination": ["box-6", 0]}},
      {"patchline": {"source": ["box-3", 0], "destination": ["box-6", 0]}},
      {"patchline": {"source": ["box-4", 0], "destination": ["box-6", 0]}},
      {"patchline": {"source": ["box-5", 0], "destination": ["box-6", 0]}},
      {"patchline": {"source": ["box-6", 0], "destination": ["box-7", 0]}},
      {"patchline": {"source": ["box-7", 0], "destination": ["box-8", 0]}},
      {"patchline": {"source": ["box-7", 1], "destination": ["box-9", 0]}},
      {"patchline": {"source": ["box-7", 2], "destination": ["box-10", 0]}},
      {"patchline": {"source": ["box-8", 0], "destination": ["box-15", 0]}},
      {"patchline": {"source": ["box-9", 0], "destination": ["box-15", 1]}},
      {"patchline": {"source": ["box-10", 0], "destination": ["box-15", 2]}},
      {"patchline": {"source": ["box-11", 0], "destination": ["box-15", 1]}},
      {"patchline": {"source": ["box-15", 0], "destination": ["box-16", 0]}},
      {"patchline": {"source": ["box-14", 0], "destination": ["box-17", 1]}},
      {"patchline": {"source": ["box-16", 0], "destination": ["box-17", 0]}},
      {"patchline": {"source": ["box-17", 0], "destination": ["box-18", 0]}},
      {"patchline": {"source": ["box-13", 0], "destination": ["box-18", 1]}},
      {"patchline": {"source": ["box-18", 0], "destination": ["box-19", 0]}},
      {"patchline": {"source": ["box-19", 0], "destination": ["box-20", 0]}},
      {"patchline": {"source": ["box-20", 0], "destination": ["box-21", 0]}},
      {"patchline": {"source": ["box-20", 0], "destination": ["box-21", 1]}}

    ]
  }
}