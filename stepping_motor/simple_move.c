	  	  //減速比15:1で200に15をかけて3000パルス/一回点
	    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
	    HAL_Delay(1000);
	    for(int i=0; i <= 3000; i++){

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
			HAL_Delay(1);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
			HAL_Delay(1);

	    }

	    HAL_Delay(1000);

	    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
	    HAL_Delay(1000);

	    for(int i=0; i <= 3000; i++){

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
			HAL_Delay(1);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
			HAL_Delay(1);


	    }
	    HAL_Delay(1000);
