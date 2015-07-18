class WorkoutsController < ApplicationController

  def view
  	@workouts = Workout.all
  end

  def show
  	@workout = Workout.find(params[:id])
  end

  def new
  	@workout = Workout.new
  end

  def delete
  	 	@workout = Workout.find(params[:id])
    if @workout.delete
      redirect_to workouts_path, notice: "Project was indeed deleted..."
    else
      redirect_to workouts_path, error: "Wat"
    end
  end

  def create
   @workout = Workout.new(workout_params)
    
    if @workout.save
      redirect_to @workout, notice: "Workout successfully created!"
    else 
      render :new
    end
  end


  private

  def workout_params
    params.require(:workout).permit(:name)
  end
end
