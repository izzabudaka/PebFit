class CreateWorkouts < ActiveRecord::Migration
  def change
    create_table :workouts do |t|
      t.string :name
      t.integer :totaltime
      t.date :lastattempt

      t.timestamps null: false
    end
  end
end
