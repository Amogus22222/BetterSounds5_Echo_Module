AudioSignalResClass {
 Inputs {
  IOPInputValueClass {
   id 22
   name "Vehicle Coverage Factor"
   tl 0 512
   children {
    13
   }
   value 2.5
  }
  IOPInputVariableClass {
   id 36
   name "GVehicleInterior"
   tl 0 672
   children {
    13
   }
   varName "GVehicleInterior"
   varResource "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf"
  }
  IOPInputVariableClass {
   id 37
   name "GCurrVehicleCoverage"
   tl 0 592
   children {
    13
   }
   varName "GCurrVehicleCoverage"
   varResource "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf"
  }
  IOPInputVariableClass {
   id 38
   name "GInterior"
   ctl 0 -24
   children {
    28
   }
   varName "GInterior"
   varResource "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf"
  }
 }
 Ops {
  IOPItemOpMaxClass {
   id 4
   name "Max 4"
   tl 768 256
   children {
    2
   }
   inputs {
    ConnectionClass "39:0" {
     id 39
     port 0
    }
    ConnectionClass "13:0" {
     id 13
     port 0
    }
   }
  }
  IOPItemOpMulClass {
   id 13
   name "Mul 13"
   tl 272 512
   children {
    4 14 23
   }
   inputs {
    ConnectionClass "36:0" {
     id 36
     port 0
    }
    ConnectionClass "37:0" {
     id 37
     port 0
    }
    ConnectionClass "22:0" {
     id 22
     port 0
    }
   }
  }
  IOPItemOpMulClass {
   id 23
   name "Mul 23"
   tl 768 768
   children {
    26
   }
   inputs {
    ConnectionClass "39:0" {
     id 39
     port 0
    }
    ConnectionClass "13:0" {
     id 13
     port 0
    }
   }
  }
  IOPItemOpInterpolateClass {
   id 26
   name "Interpolate 24"
   tl 1024 768
   children {
    27
   }
   inputs {
    ConnectionClass "23:0" {
     id 23
     port 0
    }
   }
   "Y min" 1900
   "Y max" 100
  }
  IOPItemOpInterpolateClass {
   id 28
   name "Interpolate 28"
   tl 256 0
   children {
    39
   }
   inputs {
    ConnectionClass "38:0" {
     id 38
     port 0
    }
   }
   "X min" 0.5
   "Fade In Type" "Power of 1/2"
   "Fade Out Type" "Power of 1/2"
  }
  IOPItemOpFilterClass {
   id 39
   name "Filter 39"
   tl 512 0
   children {
    4 19 23
   }
   inputs {
    ConnectionClass "28:0" {
     id 28
     port 0
    }
   }
   Frequency 10
  }
 }
 Outputs {
  IOPItemOutputClass {
   id 2
   name "BuildingVehicleInterior_W"
   tl 1280 256
   input 4
  }
  IOPItemOutputClass {
   id 14
   name "VehicleInterior_W"
   tl 1280 512
   input 13
  }
  IOPItemOutputClass {
   id 19
   name "BuildingInterior_W"
   tl 1280 0
   input 39
  }
  IOPItemOutputClass {
   id 27
   name "BuildingVehicleInterior_F"
   tl 1280 768
   input 26
  }
 }
 compiled IOPCompiledClass "{66353AB24FAECF54}" {
  visited {
   517 647 775 262 389 261 133 907 646 5 143 267 519 518 391 390 134 11 6
  }
  ins {
   IOPCompiledIn "{66353AB24FAECFB0}" {
    data {
     1 65539
    }
   }
   IOPCompiledIn "{66353AB24FAECFB8}" {
    data {
     1 458755
    }
   }
   IOPCompiledIn "{66353AB24FAECFBE}" {
    data {
     1 65539
    }
   }
   IOPCompiledIn "{66353AB24FAECFA4}" {
    data {
     1 65539
    }
   }
   IOPCompiledIn "{66353AB24FAECFAB}" {
    data {
     2 327683 458755
    }
   }
  }
  ops {
   IOPCompiledOp "{66353AB24FAECF9D}" {
    data {
     1 2 4 65537 0 393217 0
    }
   }
   IOPCompiledOp "{66353AB24FAECF86}" {
    data {
     3 3 65538 131075 6 131072 0 196608 0 0 0
    }
   }
   IOPCompiledOp "{66353AB24FAECFF3}" {
    data {
     2 196611 262147 4 393217 0 65537 0
    }
   }
   IOPCompiledOp "{66353AB24FAECFFB}" {
    data {
     1 196610 2 131073 0
    }
   }
   IOPCompiledOp "{66353AB24FAECFE3}" {
    data {
     1 262146 2 131073 0
    }
   }
   IOPCompiledOp "{66353AB24FAECFEB}" {
    data {
     1 393219 2 262144 0
    }
   }
   IOPCompiledOp "{66353AB24FAECFD0}" {
    data {
     3 3 131074 131075 2 327681 0
    }
   }
   IOPCompiledOp "{66353AB24FAECFDA}" {
    data {
     1 327682 4 65536 0 262144 0
    }
   }
  }
  outs {
   IOPCompiledOut "{66353AB24FAEC834}" {
    data {
     0
    }
   }
   IOPCompiledOut "{66353AB24FAEC83D}" {
    data {
     0
    }
   }
   IOPCompiledOut "{66353AB24FAEC820}" {
    data {
     0
    }
   }
   IOPCompiledOut "{66353AB24FAEC827}" {
    data {
     0
    }
   }
   IOPCompiledOut "{66353AB24FAEC82A}" {
    data {
     0
    }
   }
   IOPCompiledOut "{66353AB24FAEC811}" {
    data {
     0
    }
   }
  }
  processed 19
  version 2
  ins_reeval_list {
   2 3 4
  }
  ops_reeval_list {
   6
  }
 }
 Output_Order {
  ItemDetailListItemClass BuildingVehicleInterior_W {
   Name "BuildingVehicleInterior_W"
   Id 2
  }
  ItemDetailListItemClass VehicleInterior_W {
   Name "VehicleInterior_W"
   Id 14
  }
  ItemDetailListItemClass BuildingInterior_W {
   Name "BuildingInterior_W"
   Id 19
  }
  ItemDetailListItemClass BuildingVehicleInterior_F {
   Name "BuildingVehicleInterior_F"
   Id 27
  }
 }
}