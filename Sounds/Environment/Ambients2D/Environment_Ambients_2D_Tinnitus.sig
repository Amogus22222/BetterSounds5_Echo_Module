AudioSignalResClass {
 Inputs {
  IOPInputVariableClass {
   id 23
   name "Explosions_SubMix Var"
   tl -356.558 -10.291
   children {
    8
   }
   varName "Explosions_SubMix"
   varResource "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf"
  }
  IOPInputVariableClass {
   id 24
   name "Explosions_Acp Var"
   tl -356.558 93.196
   children {
    7
   }
   varName "Explosions_Acp"
   varResource "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf"
  }
  IOPInputVariableClass {
   id 25
   name "GInterior"
   tl -356.558 -393.465
   ctl 0 -24
   children {
    11
   }
   varName "GInterior"
   varResource "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf"
  }
  IOPInputVariableClass {
   id 26
   name "GIsThirdPersonCam"
   tl -356.558 -283.068
   ctl 0 -24
   children {
    13
   }
   varName "GIsThirdPersonCam"
   varResource "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf"
  }
  IOPInputVariableClass {
   id 27
   name "GCurrVehicleCoverage"
   tl -356.558 -140.157
   ctl 0 -24
   children {
    15
   }
   varName "GCurrVehicleCoverage"
   varResource "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf"
  }
 }
 Ops {
  IOPItemOpConditionClass {
   id 7
   name "Cond Distance"
   tl -105 91
   children {
    9
   }
   inputs {
    ConnectionClass "24:0" {
     id 24
     port 0
    }
   }
   "Condition Type" ">"
   Comparator 0.75
  }
  IOPItemOpConditionClass {
   id 8
   name "Cond Amplitude"
   tl -105 -10
   children {
    9
   }
   inputs {
    ConnectionClass "23:0" {
     id 23
     port 0
    }
   }
   "Condition Type" ">"
   Comparator 0.8
  }
  IOPItemOpMulClass {
   id 9
   name "Mul"
   tl 556.286 26.429
   children {
    2
   }
   inputs {
    ConnectionClass "7:0" {
     id 7
     port 0
    }
    ConnectionClass "8:0" {
     id 8
     port 0
    }
   }
  }
  IOPItemOpConditionClass {
   id 11
   name "Cond IsOutside"
   tl -108.75 -391.25
   children {
    19
   }
   inputs {
    ConnectionClass "25:0" {
     id 25
     port 0
    }
   }
   "Condition Type" "<"
   Comparator 0.5
  }
  IOPItemOpConditionClass {
   id 13
   name "Cond IsFirstPersonCamera"
   tl -106.25 -277.5
   children {
    16
   }
   inputs {
    ConnectionClass "26:0" {
     id 26
     port 0
    }
   }
  }
  IOPItemOpConditionClass {
   id 15
   name "Cond IsInVehicle"
   tl -102.5 -138.75
   children {
    16
   }
   inputs {
    ConnectionClass "27:0" {
     id 27
     port 0
    }
   }
   "Condition Type" ">"
   Comparator 0.15
  }
  IOPItemOpMulClass {
   id 16
   name "Mul IsCameraInCabin"
   tl 127.5 -208.75
   children {
    18
   }
   inputs {
    ConnectionClass "15:0" {
     id 15
     port 0
    }
    ConnectionClass "13:0" {
     id 13
     port 0
    }
   }
  }
  IOPItemOpConditionClass {
   id 18
   name "Cond IsCamerNotInCabin"
   tl 332.5 -207.5
   children {
    19
   }
   inputs {
    ConnectionClass "16:0" {
     id 16
     port 0
    }
   }
  }
  IOPItemOpMulClass {
   id 19
   name "Mul 19"
   tl 560 -288
   children {
    20
   }
   inputs {
    ConnectionClass "18:0" {
     id 18
     port 0
    }
    ConnectionClass "11:0" {
     id 11
     port 0
    }
   }
  }
 }
 Outputs {
  IOPItemOutputClass {
   id 2
   name "Tinnitus_T"
   tl 756.286 26.429
   input 9
  }
  IOPItemOutputClass {
   id 20
   name "Tinnitus_V"
   tl 751.25 -295
   input 19
  }
 }
 compiled IOPCompiledClass "{692C97150E73FA78}" {
  visited {
   517 647 389 519 779 903 261 391 1035 134 133 7 5 135 267 6
  }
  ins {
   IOPCompiledIn "{692C97150E73FA53}" {
    data {
     1 65539
    }
   }
   IOPCompiledIn "{692C97150E73FA58}" {
    data {
     1 3
    }
   }
   IOPCompiledIn "{692C97150E73FA44}" {
    data {
     1 196611
    }
   }
   IOPCompiledIn "{692C97150E73FA43}" {
    data {
     1 262147
    }
   }
   IOPCompiledIn "{692C97150E73FA4E}" {
    data {
     1 327683
    }
   }
  }
  ops {
   IOPCompiledOp "{692C97150E73FA33}" {
    data {
     1 131075 2 65536 0
    }
   }
   IOPCompiledOp "{692C97150E73FA3B}" {
    data {
     1 131075 2 0 0
    }
   }
   IOPCompiledOp "{692C97150E73FA24}" {
    data {
     1 2 4 1 0 65537 0
    }
   }
   IOPCompiledOp "{692C97150E73FA21}" {
    data {
     1 524291 2 131072 0
    }
   }
   IOPCompiledOp "{692C97150E73FA2A}" {
    data {
     1 393219 2 196608 0
    }
   }
   IOPCompiledOp "{692C97150E73FA17}" {
    data {
     1 393219 2 262144 0
    }
   }
   IOPCompiledOp "{692C97150E73FA10}" {
    data {
     1 458755 4 327681 0 262145 0
    }
   }
   IOPCompiledOp "{692C97150E73FA1D}" {
    data {
     1 524291 2 393217 0
    }
   }
   IOPCompiledOp "{692C97150E73FA18}" {
    data {
     1 65538 4 458753 0 196609 0
    }
   }
  }
  outs {
   IOPCompiledOut "{692C97150E73FAF6}" {
    data {
     0
    }
   }
   IOPCompiledOut "{692C97150E73FAF1}" {
    data {
     0
    }
   }
  }
  processed 16
  version 2
  ins_reeval_list {
   0 1 2 3 4
  }
 }
 Output_Order {
  ItemDetailListItemClass Tinnitus_T {
   Name "Tinnitus_T"
   Id 2
  }
  ItemDetailListItemClass Tinnitus_V {
   Name "Tinnitus_V"
   Id 20
  }
 }
}