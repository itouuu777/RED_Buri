	  	  //減速比15:1で200に15をかけて3000パルス/一回点
        //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);//
        HAL_Delay(50);
	    //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);//右
	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);//左
	    HAL_Delay(200);
	    for(int i=0; i < 200; i++){

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
			HAL_Delay(1);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
			HAL_Delay(1);

	    }
	    //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);//

	    HAL_Delay(100000);

	    while(1){

	    }
