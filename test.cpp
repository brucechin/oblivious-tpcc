        District res;
        bool flag = true;
        for(int j = 0; j < INT_LENGTH; j++){
            if((w_id.bits[j] == districts_[i]->d_w_id.bits[j]).reveal<bool>(PUBLIC) == false){
                flag = false;
                break;
            }
        }
        //the flag will output right result